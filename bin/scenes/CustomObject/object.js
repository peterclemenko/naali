engine.ImportExtension("qt.core");

if (!server.IsRunning())
{
    var placeholder = scene.CreateEntity();
    placeholder.GetOrCreateComponent("EC_Name");
    placeholder.SetName("TestObject"); // todo add id to name

    var placeable = placeholder.GetOrCreateComponent("EC_Placeable");
    placeable.SetPosition(0, 2, -30);
    placeable.SetScale(5.0, 5.0, 5.0);

    var customObject = placeholder.GetOrCreateComponent("EC_OgreCustomObject");

    var vertices = [
        // Front face
        float3(-1.0, -1.0,  1.0),
        float3(1.0, -1.0,  1.0),
        float3(1.0,  1.0,  1.0),
        float3(-1.0,  1.0,  1.0),

        // Back face
        float3(-1.0, -1.0, -1.0),
        float3(-1.0,  1.0, -1.0),
        float3(1.0,  1.0, -1.0),
        float3(1.0, -1.0, -1.0),
        // Top face
        float3(-1.0,  1.0, -1.0),
        float3(-1.0,  1.0,  1.0),
        float3(1.0,  1.0,  1.0),
        float3(1.0,  1.0, -1.0),

        // Bottom face
        float3(-1.0, -1.0, -1.0),
        float3(1.0, -1.0, -1.0),
        float3(1.0, -1.0,  1.0),
        float3(-1.0, -1.0,  1.0),

        // Right face
        float3(1.0, -1.0, -1.0),
        float3(1.0,  1.0, -1.0),
        float3(1.0,  1.0,  1.0),
        float3(1.0, -1.0,  1.0),

        // Left face
        float3(-1.0, -1.0, -1.0),
        float3(-1.0, -1.0,  1.0),
        float3(-1.0,  1.0,  1.0),
        float3(-1.0,  1.0, -1.0)
    ];

    var colors = [
        float3(1.0,  0.0,  0.0),    // Front face: red
        float3(1.0,  0.0,  0.0),
        float3(1.0,  0.0,  0.0),
        float3(1.0,  0.0,  0.0),

        float3(1.0,  1.0,  1.0),    // Back face: white
        float3(1.0,  1.0,  1.0),
        float3(1.0,  1.0,  1.0),
        float3(1.0,  1.0,  1.0),

        float3(0.0,  1.0,  0.0),    // Top face: green
        float3(0.0,  1.0,  0.0),
        float3(0.0,  1.0,  0.0),
        float3(0.0,  1.0,  0.0),

        float3(0.0,  0.0,  1.0),    // Bottom face: blue
        float3(0.0,  0.0,  1.0),
        float3(0.0,  0.0,  1.0),
        float3(0.0,  0.0,  1.0),

        float3(1.0,  1.0,  0.0),    // Right face: yellow
        float3(1.0,  1.0,  0.0),
        float3(1.0,  1.0,  0.0),
        float3(1.0,  1.0,  0.0),

        float3(1.0,  0.0,  1.0),    // Left face: purple
        float3(1.0,  0.0,  1.0),
        float3(1.0,  0.0,  1.0),
        float3(1.0,  0.0,  1.0)
    ];

    var indices = [
        0,  1,  2,      0,  2,  3,    // front
        4,  5,  6,      4,  6,  7,    // back
        8,  9,  10,     8,  10, 11,   // top
        12, 13, 14,     12, 14, 15,   // bottom
        16, 17, 18,     16, 18, 19,   // right
        20, 21, 22,     20, 22, 23    // left
    ]

    customObject.CreateManualObject(vertices, colors, indices, 4);
}
