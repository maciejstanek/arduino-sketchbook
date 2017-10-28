#!/usr/bin/php
<?php
if($argc < 2) {
  echo "Please provide an input file name.";
  exit(1);
}
if($argc > 2) {
  echo "Please provide only one argument.";
  exit(1);
}
if(!file_exists($argv[1])) {
  echo "This file does not exist.";
  exit(1);
}
echo base64_encode(file_get_contents($argv[1])); 
