<?php
    session_start();
    session_regenerate_id();
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
        <title>Avatar-service</title>
        <link rel="stylesheet" type="text/css" href="main.css" />
    </head>
    
    <body onLoad="document.getElementById('1').focus()">
        <div id='center'>
            <div id='topright'>
                <?php
                if($_SESSION["logged_in"]==true){
                    $user_name = $_SESSION["userName"];
                    print "<p>Welcome, $user_name</p>";
                    print "<a href='action/logoutscript.php'>Log out</a>";
                    if($_SESSION["userId"]==1 && $_SESSION["userName"]=="admin"){
                        print "&nbsp;&nbsp;<a href='adminform.php'>Moderate</a>";
                    }
                }
                ?>
            </div>
