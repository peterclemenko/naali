/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.cpp
 *  @brief  Provides Scene Structure and Assets windows and raycast drag-and-drop import of
 *          .mesh, .scene, .xml and .nbf files to the main window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureModule.h"
#include "SceneStructureWindow.h"
#include "AssetsWindow.h"
#include "SupportedFileTypes.h"
#include "AddContentWindow.h"

#include "SceneManager.h"
#include "Console.h"
#include "UiServiceInterface.h"
#include "Input.h"
#include "RenderServiceInterface.h"
#include "SceneImporter.h"
#include "EC_OgreCamera.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "NaaliUi.h"
#include "NaaliGraphicsView.h"
#include "NaaliMainWindow.h"
#include "LoggingFunctions.h"
#include "SceneDesc.h"

#include <OgreEntity.h>
#include <OgreMesh.h>

DEFINE_POCO_LOGGING_FUNCTIONS("SceneStructure");

#ifdef ASSIMP_ENABLED
#include <OpenAssetImport.h>
#endif

//#include <OgreCamera.h>

#include "MemoryLeakCheck.h"

SceneStructureModule::SceneStructureModule() :
    IModule("SceneStructure"),
    sceneWindow(0),
    assetsWindow(0)
{
}

SceneStructureModule::~SceneStructureModule()
{
    SAFE_DELETE(sceneWindow);
}

void SceneStructureModule::PostInitialize()
{
    framework_->Console()->RegisterCommand("scenestruct", "Shows the Scene Structure window.", this, SLOT(ShowSceneStructureWindow()));
    framework_->Console()->RegisterCommand("assets", "Shows the Assets window.", this, SLOT(ShowAssetsWindow()));

    inputContext = framework_->GetInput()->RegisterInputContext("SceneStructureInput", 90);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));

    connect(framework_->Ui()->GraphicsView(), SIGNAL(DragEnterEvent(QDragEnterEvent *)), SLOT(HandleDragEnterEvent(QDragEnterEvent *)));
    connect(framework_->Ui()->GraphicsView(), SIGNAL(DragMoveEvent(QDragMoveEvent *)), SLOT(HandleDragMoveEvent(QDragMoveEvent *)));
    connect(framework_->Ui()->GraphicsView(), SIGNAL(DropEvent(QDropEvent *)), SLOT(HandleDropEvent(QDropEvent *)));
}

QList<Scene::Entity *> SceneStructureModule::InstantiateContent(const QString &filename, Vector3df worldPos, bool clearScene)
{
    return InstantiateContent(filename, worldPos, SceneDesc(), clearScene);
}

QList<Scene::Entity *> SceneStructureModule::InstantiateContent(const QString &filename, Vector3df worldPos, const SceneDesc &desc, bool clearScene)
{
    return InstantiateContent(QStringList(QStringList() << filename), worldPos, desc, clearScene);
}

QList<Scene::Entity *> SceneStructureModule::InstantiateContent(const QStringList &filenames, Vector3df worldPos, const SceneDesc &desc, bool clearScene)
{
    QList<Scene::Entity *> ret;

    const Scene::ScenePtr &scene = framework_->GetDefaultWorldScene();
    if (!scene)
    {
        LogError("Could not retrieve default world scene.");
        return ret;
    }

    QList<SceneDesc> sceneDescs;

    foreach(QString filename, filenames)
    {
        if (!IsSupportedFileType(filename))
        {
            LogError("Unsupported file extension: " + filename.toStdString());
            continue;
        }

        if (filename.endsWith(cOgreSceneFileExtension, Qt::CaseInsensitive))
        {
            //boost::filesystem::path path(filename.toStdString());
            //std::string dirname = path.branch_path().string();

            TundraLogic::SceneImporter importer(scene);
//            sceneDesc = importer.GetSceneDescForScene(filename);
            sceneDescs.append(importer.GetSceneDescForScene(filename));
/*
            ret = importer.Import(filename.toStdString(), dirname, "./data/assets",
                Transform(worldPos, Vector3df(), Vector3df(1,1,1)), AttributeChange::Default, clearScene, true, false);
            if (ret.empty())
                LogError("Import failed");
            else
                LogInfo("Import successful. " + ToString(ret.size()) + " entities created.");
*/
        }
        else if (filename.endsWith(cOgreMeshFileExtension, Qt::CaseInsensitive))
        {
//            boost::filesystem::path path(filename.toStdString());
//            std::string dirname = path.branch_path().string();

            TundraLogic::SceneImporter importer(scene);
            if (IsUrl(filename))
                sceneDescs.append(importer.GetSceneDescForMesh(QUrl(filename)));
            else
                sceneDescs.append(importer.GetSceneDescForMesh(filename));
//            sceneDesc = importer.GetSceneDescForMesh(filename);
/*
            Scene::EntityPtr entity = importer.ImportMesh(filename.toStdString(), dirname, "./data/assets",
                Transform(worldPos, Vector3df(), Vector3df(1,1,1)), std::string(), AttributeChange::Default, true);
            if (entity)
                ret << entity.get();

            return ret;
*/
        }
        else if (filename.toLower().indexOf(cTundraXmlFileExtension) != -1 && filename.toLower().indexOf(cOgreMeshFileExtension) == -1)
        {
//        ret = scene->LoadSceneXML(filename.toStdString(), clearScene, false, AttributeChange::Replicate);
//            sceneDesc = scene->GetSceneDescFromXml(filename);
            sceneDescs.append(scene->GetSceneDescFromXml(filename));
        }
        else if (filename.toLower().indexOf(cTundraBinFileExtension) != -1)
        {
//        ret = scene->CreateContentFromBinary(filename, true, AttributeChange::Replicate);
//            sceneDesc = scene->GetSceneDescFromBinary(filename);
            sceneDescs.append(scene->GetSceneDescFromXml(filename));
        }
        else
        {
#ifdef ASSIMP_ENABLED
            boost::filesystem::path path(filename.toStdString());
            AssImp::OpenAssetImport assimporter;
            QString extension = QString(path.extension().c_str()).toLower();
            if (assimporter.IsSupportedExtension(extension))
            {
                std::string dirname = path.branch_path().string();
                std::vector<AssImp::MeshData> meshNames;
                assimporter.GetMeshData(filename, meshNames);

                TundraLogic::SceneImporter sceneimporter(scene);
                for (size_t i=0 ; i<meshNames.size() ; ++i)
                {
                    Scene::EntityPtr entity = sceneimporter.ImportMesh(meshNames[i].file_.toStdString(), dirname, meshNames[i].transform_,
                        std::string(), "local://", AttributeChange::Default, false, meshNames[i].name_.toStdString());
                    if (entity)
                        ret.append(entity.get());
                }

                return ret;
            }
#endif
        }
    }

    if (!sceneDescs.isEmpty())
    {
        AddContentWindow *addContent = new AddContentWindow(framework_, scene);
//        addContent->AddDescription(sceneDesc);
        addContent->AddDescription(sceneDescs[0]);
        if (worldPos != Vector3df())
            addContent->AddPosition(worldPos);
        addContent->show();
    }

    return ret;
}

void SceneStructureModule::CentralizeEntitiesTo(const Vector3df &pos, const QList<Scene::Entity *> &entities)
{
    Vector3df minPos(1e9f, 1e9f, 1e9f);
    Vector3df maxPos(-1e9f, -1e9f, -1e9f);

    foreach(Scene::Entity *e, entities)
    {
        EC_Placeable *p = e->GetComponent<EC_Placeable>().get();
        if (p)
        {
            Vector3df pos = p->transform.Get().position;
            minPos.x = std::min(minPos.x, pos.x);
            minPos.y = std::min(minPos.y, pos.y);
            minPos.z = std::min(minPos.z, pos.z);
            maxPos.x = std::max(maxPos.x, pos.x);
            maxPos.y = std::max(maxPos.y, pos.y);
            maxPos.z = std::max(maxPos.z, pos.z);
        }
    }

    // We assume that world's up axis is Z-coordinate axis.
    Vector3df importPivotPos = Vector3df((minPos.x + maxPos.x) / 2, (minPos.y + maxPos.y) / 2, minPos.z);
    Vector3df offset = pos - importPivotPos;

    foreach(Scene::Entity *e, entities)
    {
        EC_Placeable *p = e->GetComponent<EC_Placeable>().get();
        if (p)
        {
            Transform t = p->transform.Get();
            t.position += offset;
            p->transform.Set(t, AttributeChange::Default);
        }
    }
}

bool SceneStructureModule::IsSupportedFileType(const QString &fileRef)
{
    if (fileRef.endsWith(cTundraXmlFileExtension, Qt::CaseInsensitive) ||
        fileRef.endsWith(cTundraBinFileExtension, Qt::CaseInsensitive) ||
        fileRef.endsWith(cOgreMeshFileExtension, Qt::CaseInsensitive) ||
        fileRef.endsWith(cOgreSceneFileExtension, Qt::CaseInsensitive))
    {
        return true;
    }
    else
    {
#ifdef ASSIMP_ENABLED
        boost::filesystem::path path(fileRef.toStdString());
        AssImp::OpenAssetImport assimporter;
        QString extension = QString(path.extension().c_str()).toLower();
        if (assimporter.IsSupportedExtension(extension))
            return true;
#endif
        return false;
    }
}

bool SceneStructureModule::IsMaterialFile(const QString &fileRef)
{
    if (fileRef.toLower().endsWith(".material"))
        return true;
    return false;
}

bool SceneStructureModule::IsUrl(const QString &fileRef)
{
    if (fileRef.startsWith("http://") || fileRef.startsWith("https://"))
        return true;
    return false;
}

void SceneStructureModule::CleanReference(QString &fileRef)
{
    if (!IsUrl(fileRef))
    {
        QUrl fileUrl(fileRef);
        fileRef = fileUrl.path();
#ifdef _WINDOWS
        // We have '/' as the first char on windows and the filename
        // is not identified as a file properly. But on other platforms the '/' is valid/required.
        fileRef = fileRef.mid(1);
#endif
    }
}

void SceneStructureModule::ShowSceneStructureWindow()
{
    /*UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    if (!ui)
        return;*/

    if (sceneWindow)
    {
        //ui->ShowWidget(sceneWindow);
        sceneWindow->show();
        return;
    }

    NaaliUi *ui = GetFramework()->Ui();
    if (!ui)
        return;

    sceneWindow = new SceneStructureWindow(framework_, ui->MainWindow());
    sceneWindow->setWindowFlags(Qt::Tool);
    sceneWindow->SetScene(framework_->GetDefaultWorldScene());
    sceneWindow->show();

    //ui->AddWidgetToScene(sceneWindow);
    //ui->ShowWidget(sceneWindow);
}

void SceneStructureModule::ShowAssetsWindow()
{
    /*UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    if (!ui)
        return;*/

    if (assetsWindow)
    {
        //ui->ShowWidget(assetsWindow);
        assetsWindow->show();
        return;
    }

    NaaliUi *ui = GetFramework()->Ui();
    if (!ui)
        return;

    assetsWindow = new AssetsWindow(framework_, ui->MainWindow());
    assetsWindow->setWindowFlags(Qt::Tool);
    assetsWindow->show();

    //ui->AddWidgetToScene(assetsWindow);
    //ui->ShowWidget(assetsWindow);
}

void SceneStructureModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    Input &input = *framework_->GetInput();

    const QKeySequence &showSceneStruct = input.KeyBinding("ShowSceneStructureWindow", QKeySequence(Qt::ShiftModifier + Qt::Key_S));
    const QKeySequence &showAssets = input.KeyBinding("ShowAssetsWindow", QKeySequence(Qt::ShiftModifier + Qt::Key_A));

    QKeySequence keySeq(e->keyCode | e->modifiers);
    if (keySeq == showSceneStruct)
        ShowSceneStructureWindow();
    if (keySeq == showAssets)
        ShowAssetsWindow();
}

void SceneStructureModule::HandleDragEnterEvent(QDragEnterEvent *e)
{
    // If at least one file is supported, accept.
    bool accept = false;
    if (e->mimeData()->hasUrls())
    {   
        foreach(QUrl url, e->mimeData()->urls())
        {
            if (IsSupportedFileType(url.path()))
                accept = true;
            // Accept .material only if a single material is being dropped
            else if (IsMaterialFile(url.path()))
                if (e->mimeData()->urls().count() == 1)
                    accept = true;
        }
    }
    e->setAccepted(accept);
}

void SceneStructureModule::HandleDragMoveEvent(QDragMoveEvent *e)
{
    // If at least one file is supported, accept.
    if (!e->mimeData()->hasUrls())
        return;
    foreach(QUrl url, e->mimeData()->urls())
    {
        if (IsSupportedFileType(url.path()))
            e->accept();
        else if (IsMaterialFile(url.path()))
        {
            // Raycast to see if there is a submesh under the material drop
            Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
            if (renderer)
            {
                RaycastResult* res = renderer->Raycast(e->pos().x(), e->pos().y());
                if (res->entity_)
                {
                    EC_Mesh *mesh = res->entity_->GetComponent<EC_Mesh>().get();
                    if (mesh)
                    {
                        e->accept();
                        return;
                    }
                }
            }                
            e->ignore();
        }
    }
}

void SceneStructureModule::HandleDropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        // Handle materials with own handler
        if (e->mimeData()->urls().count() == 1)
        {
            QString fileRef = e->mimeData()->urls().first().toString();
            if (IsMaterialFile(fileRef))
            {
                CleanReference(fileRef);
                HandleMaterialDropEvent(e, fileRef);
                return;
            }
        }

        // Handle other supperted file types
        QList<Scene::Entity *> importedEntities;

        Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
        if (!renderer)
            return;

        Vector3df worldPos;
        RaycastResult* res = renderer->Raycast(e->pos().x(), e->pos().y());
        if (!res->entity_)
        {
            // No entity hit, use camera's position with hard-coded offset.
            const Scene::ScenePtr &scene = framework_->GetDefaultWorldScene();
            if (!scene)
                return;

            foreach(Scene::EntityPtr cam, scene->GetEntitiesWithComponent(EC_OgreCamera::TypeNameStatic()))
                if (cam->GetComponent<EC_OgreCamera>()->IsActive())
                {
                    EC_Placeable *placeable = cam->GetComponent<EC_Placeable>().get();
                    if (placeable)
                    {
                        //Ogre::Ray ray = cam->GetComponent<EC_OgreCamera>()->GetCamera()->getCameraToViewportRay(e->pos().x(), e->pos().y());
                        Quaternion q = placeable->GetOrientation();
                        Vector3df v = q * -Vector3df::UNIT_Z;
                        //Ogre::Vector3 oV = ray.getPoint(20);
                        worldPos = /*Vector3df(oV.x, oV.y, oV.z);*/ placeable->GetPosition() + v * 20;
                        break;
                    }
                }
        }
        else
            worldPos = res->pos_;

        foreach (QUrl url, e->mimeData()->urls())
        {
            QString fileRef = url.toString();
            CleanReference(fileRef);
            importedEntities.append(InstantiateContent(fileRef, worldPos/*Vector3df()*/, false));
        }

        // Calculate import pivot and offset for new content
        //if (importedEntities.size())
        //    CentralizeEntitiesTo(worldPos, importedEntities);

        e->acceptProposedAction();
    }
}

void SceneStructureModule::HandleMaterialDropEvent(QDropEvent *e, const QString &materialRef)
{   
    // Raycast to see if there is a submesh under the material drop
    Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
    if (renderer)
    {
        RaycastResult* res = renderer->Raycast(e->pos().x(), e->pos().y());
        if (res->entity_)
        {
            EC_Mesh *mesh = res->entity_->GetComponent<EC_Mesh>().get();
            if (mesh)
            {
                uint subMeshCount = mesh->GetNumSubMeshes();
                uint subMeshIndex = res->submesh_;
                if (subMeshIndex < subMeshCount)
                {
                    materialDropData.affectedIndexes.clear();

                    // Get the filename
                    QString matName = materialRef;
                    matName.replace("\\", "/");
                    matName = matName.split("/").last();

                    QString cleanMaterialRef = matName;

                    // Add our dropped material to the raycasted submesh,
                    // append empty string or the current material string to the rest of them
                    AssetReferenceList currentMaterials = mesh->getmeshMaterial();
                    AssetReferenceList afterMaterials;
                    for(uint i=0; i<subMeshCount; ++i)
                    {
                        if (i == subMeshIndex)
                        {
                            afterMaterials.Append(cleanMaterialRef);
                            materialDropData.affectedIndexes.append(i);
                        }
                        else if (i < currentMaterials.Size())
                            afterMaterials.Append(currentMaterials[i]);
                        else
                            afterMaterials.Append(AssetReference());
                    }
                    // Clear our any empty ones from the end
                    for(uint i=afterMaterials.Size(); i>0; --i)
                    {
                        AssetReference assetRef = afterMaterials[i-1];
                        if (assetRef.ref.isEmpty())
                            afterMaterials.RemoveLast();
                        else
                            break;
                    }

                    // Url: Finish now
                    // File: Finish when add content dialog gives Completed signal
                    materialDropData.mesh = mesh;
                    materialDropData.materials = afterMaterials;

                    if (IsUrl(materialRef))
                    {
                        QString baseUrl = materialRef.left(materialRef.length() - cleanMaterialRef.length());
                        FinishMaterialDrop(true, baseUrl);
                    }
                    else
                    {
                        const Scene::ScenePtr &scene = framework_->GetDefaultWorldScene();
                        if (!scene)
                        {
                            LogError("Could not retrieve default world scene.");
                            return;
                        }

                        // Open source file for reading
                        QFile materialFile(materialRef);
                        if (!materialFile.open(QIODevice::ReadOnly))
                        {
                            LogError("Could not open dropped material file.");
                            return;
                        }

                        // Create scene description
                        SceneDesc sceneDesc;
                        sceneDesc.type = SceneDesc::AssetUpload;
                        sceneDesc.filename = materialRef;

                        // Add our material asset to scene description
                        AssetDesc ad;
                        ad.typeName = "material";
                        ad.source = materialRef;
                        ad.destinationName = matName;
                        ad.data = materialFile.readAll();
                        ad.dataInMemory = true;

                        sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
                        materialFile.close();

                        // Add texture assets to scene description
                        TundraLogic::SceneImporter importer(scene);
                        QSet<QString> textures = importer.ProcessMaterialForTextures(ad.data);
                        if (!textures.empty())
                        {
                            QString dropFolder = materialRef;
                            dropFolder = dropFolder.replace("\\", "/");
                            dropFolder = dropFolder.left(dropFolder.lastIndexOf("/")+1);

                            foreach(QString textureName, textures)
                            {
                                AssetDesc ad;
                                ad.typeName = "texture";
                                ad.source = dropFolder + textureName;
                                ad.destinationName = textureName;
                                ad.dataInMemory = false;
                                sceneDesc.assets[qMakePair(ad.source, ad.subname)] = ad;
                            }
                        }

                        // Show add content window
                        AddContentWindow *addMaterials = new AddContentWindow(framework_, scene);
                        connect(addMaterials, SIGNAL(Completed(bool, const QString&)), SLOT(FinishMaterialDrop(bool, const QString&)));
                        addMaterials->AddDescription(sceneDesc);
                        addMaterials->show();
                    }
                    e->acceptProposedAction();
                }
            }
        }
    }
}

void SceneStructureModule::FinishMaterialDrop(bool apply, const QString &materialBaseUrl)
{
    if (apply)
    {
        EC_Mesh *mesh = materialDropData.mesh;
        if (mesh)
        {
            // Inspect the base url where the assets were uploaded
            // rewrite the affeced materials to have the base url in front
            if (!materialDropData.affectedIndexes.empty())
            {
                AssetReferenceList rewrittenMats;
                AssetReferenceList mats = materialDropData.materials;
                for(uint i=0; i<mats.Size(); ++i)
                {
                    if (materialDropData.affectedIndexes.contains(i))
                    {
                        QString newRef = materialBaseUrl;
                        if (!newRef.endsWith("/")) // just to be sure
                            newRef.append("/");
                        newRef.append(mats[i].ref);
                        rewrittenMats.Append(newRef);
                    }
                    else
                        rewrittenMats.Append(mats[i].ref);

                }
                mesh->setmeshMaterial(rewrittenMats);
            }
        }
    }
    materialDropData.mesh = 0;
    materialDropData.materials = AssetReferenceList();
    materialDropData.affectedIndexes.clear();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(SceneStructureModule)
POCO_END_MANIFEST
