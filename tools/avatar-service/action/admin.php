<?php
session_start();
if(!$_SESSION["userId"]==1 && !$_SESSION["userName"]=="admin"){
    header("location: index.php");
}
include_once "dbconnect.php";
$selection = $_POST['mode'];

if($selection == 'addavatar'){ //add avatar
    $avatarname = $_POST['avatarname'];
    $avatarscale = $_POST['avatarscale'];

    $filename_model = $_FILES["modelfile"]["name"];
    $filesize_model = $_FILES["modelfile"]["size"];

    $filesize_limit = 5000000;  //max file size 5mb

    $uploadDir = "../scene/models/";  //write perm for apache
    $storageDir = $uploadDir . $avatarname . "/";
    $modelDir = $storageDir . "models/";
    $imageDir = $storageDir . "images/";

    //create directory
    mkdir($storageDir);


    function isAllowedExtension($fileName, $mode) {
        $allowedExtensions_model = array("dae");
        $allowedExtensions_texture = array("png", "jpg");

        if ($mode == 1){
            return in_array(end(explode(".", strtolower($fileName))), $allowedExtensions_model);
        }
        else if ($mode == 2){
            return in_array(end(explode(".", strtolower($fileName))), $allowedExtensions_texture);
        }
    }
    if (isAllowedExtension($filename_model, 1) && $filesize_model <= $filesize_limit){
        mkdir($modelDir);
        $tmpname = $_FILES["modelfile"]["tmp_name"];

        if ($_FILES["modelfile"]["error"] > 0){
            echo "Return Code: " . $_FILES["modelfile"]["error"] . "<br />";
        }
        else{
            if (file_exists($modelDir . $filename_model)){
                echo $filename_model . " already exists<br />";
            }
            else{
                move_uploaded_file($tmpname, $modelDir . $filename_model);
                echo "Stored in: " . $modelDir . $filename_model . "<br />";

                $query = "INSERT INTO avatar(avatarName, avatarScale, avatarFile) VALUES('$avatarname', '$avatarscale', '$filename_model')";
                $result = mysql_query($query);
                if(!$result){
                    print mysql_error();
                    mysql_close($dbConnection);
                    exit;
                }
                else{
                    print "Avatar model successfully added<br />";
                }
            }
        }
    }
    else if (isAllowedExtension($filename_model, 1) && $filesize_model > $filesize_limit){
        print "Model file is too big to upload<br />";
    }
    else{
        print "Invalid model file<br />";
    }
    //multiple texture upload
    foreach($_FILES["texturefile"]["name"] as $key => $value){
        mkdir($imageDir);
        $filename_texture = $_FILES["texturefile"]["name"][$key];
        $filesize_texture = $_FILES["texturefile"]["size"][$key];
        $tmpname2 = $_FILES["texturefile"]["tmp_name"][$key];

        if (isAllowedExtension($filename_texture, 2) && $filesize_texture <= $filesize_limit){
            if ($_FILES["texturefile"]["error"][$key] > 0){
                echo "Return Code: " . $_FILES["texturefile"]["error"][$key] . "<br />";
            }
            else{
                if (file_exists($imageDir . $filename_texture)){
                    echo $filename_texture . " already exists<br />";
                }
                else{
                    move_uploaded_file($tmpname2, $imageDir . $filename_texture);
                    echo "Stored in: " . $imageDir . $filename_texture . "<br />";
                }
            }
        }
        else if (isAllowedExtension($filename_texture, 2) && $filesize_texture > $filesize_limit){
            print "Texture file is too big to upload";
        }
        else{
            print "<br />Invalid texture file";
        }
    }
    print "<br /><a href='../adminform.php'>Back</a>";
}

//remove avatar
else if($selection == 'removeavatar'){
    $avatarid = $_POST['drbavatar'];
    if($avatarid != 0){
        //del useravatar links
        $query1 = "DELETE FROM useravatar WHERE avatarId='$avatarid'";
        $result1 = mysql_query($query1);
        if(!$result1){
            print mysql_error();
            mysql_close($dbConnection);
            exit;
        }
        //del avatar files
        $query3 = "SELECT * FROM avatar WHERE avatarId='$avatarid'";
        $result3 = mysql_query($query3);
        if(!$result3){
            print mysql_error();
            mysql_close($dbConnection);
            exit;
        }
        $avatarfile = mysql_fetch_assoc($result3);
        #$folder = "../models/" . $avatarfile['avatarName'] . "/models/";
        #print $folder . $avatarfile['avatarFile'];
        #unlink($folder . $avatarfile['avatarFile']);
        $modelfolder = "../scene/models/" . $avatarfile['avatarName'] . "/";
        
        $d = $modelfolder . "models/";
        foreach(glob($d.'*.*') as $v){
            unlink($v);
        }
        $d2 = $modelfolder . "images/";
        foreach(glob($d2.'*.*') as $v2){
            unlink($v2);
        }
        rmdir($d);
        rmdir($d2);
        rmdir($modelfolder);        
        
        //check if texture with the same name exists
        //check for (avatarfile - suffix), delete this .png
        //del avatar from db
        $query2 = "DELETE FROM avatar WHERE avatarId='$avatarid'";
        $result2 = mysql_query($query2);
        if(!$result2){
            print mysql_error();
            mysql_close($dbConnection);
            exit;
        }
        if($result1 && $result2){
            print "<br />operation successful. <a href='../adminform.php'>Back</a>";
        }   
    }
    else{
        print "<br />no avatar selected. <a href='../adminform.php'>Back</a>";
    }
}

//user stuff
else if($selection == 'user'){ 
    $action = $_POST['action'];
    $userid = $_POST['drbuser'];
    if($userid != 0){
        if($action == 'remove'){
            //del useravatar info
            $query1 = "DELETE FROM useravatar WHERE userId='$userid'";
            $result1 = mysql_query($query1);
            if(!$result1){
                print mysql_error();
                mysql_close($dbConnection);
                exit;
            }
            //del user
            $query2 = "DELETE FROM user WHERE userId='$userid'";
            $result2 = mysql_query($query2);
            if(!$result2){
                print mysql_error();
                mysql_close($dbConnection);
                exit;
            }
            if($result1 && $result2){
                print "<br />operation successful. <a href='../adminform.php'>Back</a>";
            }
        }
        if($action == 'edit'){
            $newname = mysql_real_escape_string(strip_tags($_POST['newname']));
            $newpass = crypt(mysql_real_escape_string(strip_tags($_POST['newpassword'])));
            
            $cbname = $_POST['chkname'];
            $cbpass = $_POST['chkpass'];
            if($cbname && !$cbpass){
                $query = "UPDATE user SET userName='$newname' WHERE userId='$userid'";
            }
            if(!$cbname && $cbpass){
                $query = "UPDATE user SET userPassword='$newpass' WHERE userId='$userid'";
            }
            if($cbname && $cbpass){
                $query = "UPDATE user SET userName='$newname', userPassword='$newpass' WHERE userId='$userid'";
            }
            if(!$cbname && !$cbpass){
                print "<br />no fields selected <a href='../adminform.php'>Back</a>";
            }
            
            $result = mysql_query($query);
            if(!$result){
                print mysql_error();
                mysql_close($dbConnection);
                exit;
            }
            else{
                print "<br />operation successful. <a href='../adminform.php'>Back</a>";
            }
        }
    }
}
?>
