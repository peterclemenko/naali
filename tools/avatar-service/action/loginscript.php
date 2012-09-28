<?php
session_start();
if($_SESSION["logged_in"]==true){
    header("location:../avatar.php");
}

include_once "dbconnect.php";
$username = mysql_real_escape_string(strip_tags($_POST["txtUsername"]));
$password = mysql_real_escape_string(strip_tags($_POST["txtPassword"]));

if(empty($username) || empty($password)){
    header("location:../index.php");
}
else{
    $query = "SELECT userName, userPassword FROM user WHERE userName = '$username'";
    $result = mysql_query($query);
    if(!$result){
        $error=mysql_error();
        print $error;
        mysql_close($dbConnection);
        exit;
    }

    $dataArray = mysql_fetch_assoc($result);
    $db_user_name = $dataArray[userName];
    $db_user_password = $dataArray[userPassword];
    if (strcmp($username,$db_user_name)==0 && crypt($password,$db_user_password)==$db_user_password)
    {
        $_SESSION["logged_in"]=true;
        $_SESSION["userName"]=$db_user_name;
        //if ($rememberMe==1)
        //{
        //    setcookie("username",$username,time() + 60*60*24*365);
        //    setcookie("password",$password,time() + 60*60*24*365);
        //    setcookie("rememberme",$rememberMe,time() + 60*60*24*365);
        //}
        $query = "SELECT avatarId FROM useravatar WHERE userId IN (SELECT userId FROM user WHERE userName = '$db_user_name')";
        $result = mysql_query($query);
        if(!$result){
            $error=mysql_error();
            print $error;
            mysql_close($dbConnection);
            exit;
        }
        $dataArray = mysql_fetch_assoc($result);
        $_SESSION["avatarId"]=$dataArray['avatarId'];

        $query = "SELECT userId FROM user WHERE userName = '$db_user_name'";
        $result = mysql_query($query);
        if(!$result){
            $error=mysql_error();
            print $error;
            mysql_close($dbConnection);
            exit;
        }
        $dataArray = mysql_fetch_assoc($result);
        $_SESSION["userId"]=$dataArray['userId'];

        header("location:../avatar.php?avatar=" . $_SESSION["avatarId"]);
    }
    else
    {
        header("location:../index.php");
    }
}

?>
