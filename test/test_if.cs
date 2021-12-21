<html>
<head>
</head>

<body>
<h1>Welcome</h1>
<?cs if : 2   +3== -5 ?>
  <?cs var: New.name?>
<?cs elif : 100 * 25.5 >= 22 ?>
  sbsbsbsb
<?cs elif :  New.name == "1" ?>
  newman
<?cs else ?>
  <?cs var: desc?>
<?cs /if?>



<?cs if:Me.0.name=="ml" ?>
test2222
<?cs /if?>

<?cs if: Count>100&&?New.name ?>
if3333
<?cs elif: Count==100||(Count==200&&?New.name) ?>
else3333
<?cs /if?>
</body>
</html>
