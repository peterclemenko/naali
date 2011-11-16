<?php
session_start();
if($_SESSION["logged_in"]==false){
    header("location:../index.php");
}
include_once "dbconnect.php";

$userid = $_SESSION["userId"];
$username = $_SESSION["userName"];
$avatarid = $_SESSION["selectedAvatarId"];

print $userid;
print $username;
print $avatarid;
//check if user has a record yet
$hasrecord = mysql_query("SELECT COUNT(*) FROM useravatar WHERE userId='$userid'");
if(!$hasrecord){
    print mysql_error();
    mysql_close($dbConnection);
    exit;
}
$dataArray = mysql_fetch_assoc($hasrecord);

//check if record exists, update or insert as result
if($dataArray['COUNT(*)'] == 0){
    $query = "INSERT INTO useravatar(userId, avatarId) VALUES($userid, $avatarid)";
}
else{
    $query = "UPDATE useravatar SET userId='$userid', avatarId='$avatarid' WHERE userId='$userid'";
}
$result = mysql_query($query);
if(!$result){
    print mysql_error();
    mysql_close($dbConnection);
    exit;
    print "<a href='../index.php'>Back</a>";
}
else{
    header("location: ../avatar.php?avatar=$avatarid");
}

?>
