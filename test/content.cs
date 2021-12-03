<h3><?cs var: desc ?></h3>
<?cs each: image = images ?>
     <img src="<?cs var: image ?>" />
     <?cs each: user=users?>
         <h5><?cs var: user.name?><h5>
         <h5><?cs var: user.score ?><h5>
     <?cs /each ?>
<?cs /each?>
