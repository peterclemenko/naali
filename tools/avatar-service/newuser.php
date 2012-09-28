<?php include_once "top.php"; ?>

<?php
if($_SESSION["logged_in"]==true){
    $avatarId = $_SESSION['selectedAvatarId'];
    header("location:avatar.php?avatar=" . $avatarId);
}
else {
echo "
<script type='text/javascript'>
    function checkValues(txtUsername, txtEmail, txtPassword){
        username = document.forms[0].txtUsername.value 
        email = document.forms[0].txtEmail.value
        password = document.forms[0].txtPassword.value

        if (username == '' || username == null || !isNaN(username) || username.charAt(0) == ' ')
        {
            alert('Username is required');
            document.getElementById('1').focus();
            return false;
        }
        else if (email == '' || email == null || !isNaN(email) || email.charAt(0) == ' ')
        {
            alert('Email is required');
            document.getElementById('2').focus();
            return false;
        }
        else if (password == '' || password == null || !isNaN(password) || password.charAt(0) == ' ')
        {
            alert('Password is required');
            document.getElementById('3').focus();
            return false;
        }
        return true;
    }
</script>
";

echo "
<h3>Create a new user</h3>
<br />
<fieldset>
<form name='input' action='action/createuser.php' method='POST' onsubmit='return checkValues(this);'>
<center><table>
    <tr><td>User:</td><td><input type='text' id='1' name='txtUsername' /></td></tr>
    <tr><td>Email:</td><td><input type='text' id='2' name='txtEmail' /></td></tr>
    <tr><td>Password:</td><td><input type='password' id='3' name='txtPassword' /></td></tr>
<table></center>
<br />
<table>
<tr><td><input type='submit' value='Submit' /></td>
<td><input type='button' value='Cancel' onclick='window.location.href=&quot;index.php&quot;'></td></tr>
</table>
</form>
</fieldset>
";
}
?>
<?php include_once "bottom.php"; ?>
