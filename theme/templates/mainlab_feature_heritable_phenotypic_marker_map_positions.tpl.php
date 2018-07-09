<?php

/*
 * File: mainlab_feature_heritable_phenotypic_marker_map_positions.tpl.php
 * Copied from MainLab Tripal extension module for Tripal that includes custom tables for Chado used
 * by the Main Bioinformatics Laboratory. Version core = 7.x.
 * Modified for tripal_map to display link to MapViewer page from the feature Map Positions pane.
 */

$feature = $variables['node']->feature;
$map_positions = $feature->mainlab_mtl->map_positions;
$counter_pos = count($map_positions);

if ($counter_pos > 0) {
  $header = array ('#', 'Map Name', 'Linkage Group', 'Bin', 'Chromosome', 'Position', 'MapViewer', 'CMap');
  $rows = array ();
  $counter = 1; 
  foreach($map_positions AS $pos) {
    $map = $pos->nid ? "<a href=\"/node/$pos->nid\">$pos->name</a>" : $pos->name;
    $lg = $pos->linkage_group ? $pos->linkage_group : "N/A";
    $bin = $pos->bin ? $pos->bin : "N/A"; 
    $chr = $pos->chr ?$pos->chr : "N/A";
    $start = number_format($pos->mtl_start, 1) == 0 ? "N/A" : number_format($pos->mtl_start, 1);
    $highlight = $node->feature->uniquename;
    $linkage_group = $pos->linkage_group ? $pos->linkage_group : "";
    $mtl_name = $highlight ? $highlight : "";
    
    $mapviewer = (!$pos->id || !$pos->linkage_group)? "N/A" : "<a href=\"/mapviewer/$pos->id".
    "/$linkage_group/$mtl_name\" target=\"_blank\">View</a>";
    $cmap = (!$pos->urlprefix || !$pos->accession)? "N/A" : "<a href=\"$pos->urlprefix$pos->accession" . 
    "&ref_map_acc=-1&highlight=" . $highlight . "\">View</a>";
    $rows[] = array ($counter, $map, $lg, $bin, $chr, $start, $mapviewer, $cmap);
    $counter ++;
  }
  $table = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(
      'id' => 'tripal_feature_mtl-table-map-positions',
    ),
    'sticky' => FALSE,
    'caption' => '',
    'colgroups' => array(),
    'empty' => '',
  );
  print theme_table($table);
} ?>
