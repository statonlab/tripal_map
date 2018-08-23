<?php

/*
 * File: mainlab_feature_genetic_marker_map_positions.tpl.php
 * From MainLab Tripal extension module that includes custom tables for Chado used
 * by the Main Bioinformatics Laboratory. Version core = 7.x.
 * Modified for tripal_map to display link to MapViewer page from the feature Map Positions pane.
 */

$feature = $variables['node']->feature;
$map_positions = property_exists($feature, 'map_positions') ? $feature->map_positions : array();
$counter_pos = count($map_positions);
 if ($counter_pos > 0) {
   $hasChr = false;
   foreach($map_positions AS $pos) {
     if ($pos->chr) {
       $hasChr = true;
     }
   }
  $header = $hasChr ? array ('#', 'Map Name', 'Linkage Group', 'Bin', 'Chromosome', 'Position', 'Locus', 'MapViewer', 'CMap') : 
  array ('#', 'Map Name', 'Linkage Group', 'Bin', 'Position', 'Locus', 'MapViewer', 'CMap');
  $rows = array ();
  $counter = 1; 

  foreach($map_positions AS $pos) {
    $map = $pos->nid ? "<a href=\"/node/$pos->nid\">$pos->name</a>" : $pos->name;
    $lg = $pos->linkage_group ? $pos->linkage_group : "N/A";
    $bin = $pos->bin ? $pos->bin : "N/A";
    if ($hasChr) {
      $chr = $pos->chr ?$pos->chr : "N/A";
    }
    $position = number_format($pos->locus_start, 2);
    $locus = $pos->locus_name;
    $highlight = $node->feature->uniquename;
    $linkage_group = $pos->linkage_group; 
    $marker_name = $locus ? $locus : $highlight;
    $mapviewer = (!$pos->id || !$pos->linkage_group)? "N/A" : "<a href=\"/mapviewer/$pos->id".
    "/$linkage_group/$pos->locus_name\" target=\"_blank\">View</a>";
    $cmap = (!$pos->urlprefix || !$pos->accession)? "N/A" : "<a href=\"$pos->urlprefix$pos->accession" . 
    "&ref_map_acc=-1&highlight=" . $highlight . "\">View</a>";
    
    $rows[] = $hasChr ? array ($counter, $map, $lg, $bin, $chr, $position, $locus, $mapviewer, $cmap) : 
    array ($counter, $map, $lg, $bin, $position, $locus, $mapviewer, $cmap);
    $counter ++;
  }
  $table = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_feature_genetic_marker-table-map-positions',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );
    
  print theme_table($table);
} ?>

