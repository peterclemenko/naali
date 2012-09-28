<?php
$host = "localhost";
$username = "root";
$password = "N73J";
$dbconnection = mysql_pconnect($host, $username, $password); 
if (!$dbconnection){
    print "Failed to connect to the database.";
    exit;
}

$databaseName = "avatar";
mysql_select_db($databaseName);
if (!$databaseName){
    print "Database does not exist!";
    exit;
}
?>
