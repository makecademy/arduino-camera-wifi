<?php

 $target_path = "uploads/";

$target_path = $target_path . basename( $_FILES['picture']['name']); 

if(move_uploaded_file($_FILES['picture']['tmp_name'], $target_path)) {
    echo "The file ".  basename( $_FILES['picture']['name']). 
    " has been uploaded";

} 
else {
    echo "There was an error uploading the file, please try again!";
}

?>