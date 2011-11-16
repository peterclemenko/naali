<?php include_once "../top.php"; ?>

<?php 
include_once "dbconnect.php";

$username = mysql_real_escape_string(strip_tags($_POST["txtUsername"]));
$email = mysql_real_escape_string(strip_tags($_POST["txtEmail"]));
$password = crypt(mysql_real_escape_string(strip_tags($_POST["txtPassword"])));

if(empty($username) || empty($email) || empty($password)){
    header("location:../newuser.php");
}
else{
    $query = "INSERT INTO user(userName, userEmail, userPassword) VALUES('$username', '$email', '$password')";

    $result = mysql_query($query);
    if ($result){
        $_SESSION["logged_in"]=true;
        $_SESSION["userName"]=$username;
        
        $query = "SELECT userId FROM user WHERE userName = '$username'";
        $result = mysql_query($query);
        if(!$result){
            print mysql_error();
            mysql_close($dbConnection);
            exit;
        }
        $dataArray = mysql_fetch_assoc($result);
        $_SESSION['userId'] = $dataArray['userId'];
        header("location: ../avatar.php");
    }
    else{
        $error = mysql_error();
        print "$error <br/>";
        print "<a href='../newuser.php'>Back</a>";
    }
    
}
?>

<?php include_once "../bottom.php"; ?>
