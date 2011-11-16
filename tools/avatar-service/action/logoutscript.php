<?php
session_start();
unset($_SESSION["logged_in"]);
session_destroy();

?>
<?php
session_start();
session_regenerate_id();

header("location:../index.php");
?>

