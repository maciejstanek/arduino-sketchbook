#!/usr/bin/php
<?php

$im = base64_encode(file_get_contents('icon.png')); 
//echo '<img width="16" height="16">data:image/png;base64,'.$imdata.'</>';
//echo '<img src="data:image/png;base64,' . $im . '"/>';
echo $im;
