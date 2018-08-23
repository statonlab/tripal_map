<?php

/**
 * @file
 *	 Utilities to support creating the MapViewer form to display genetic maps
 *
 * @ingroup tripal_map
 */


/**
 * function: tripal_map_get_select_list_options
 *	 Query Chado for map, linkage group and organism data with distinct selection of tuples: (map feature id, organism id)
 *
 *  @return 
 *  An array containing the following data structures:
 * $map_org
 *   An array to which map ids are assigned, each containing a sub array with the primary organism the map
 *   is associated with, and a nested array of ids for all linkage groups belonging to
 *   each map. Used to supply organism associated with each map, and provide default linkage
 *   group, when only a map name is supplied to the mapviewer url path.
 *	 Example structure: $map_org => [225 (map_id): [organism_id: 13, linkage_groups [LG1 (lg_name): 1435102( lg_id), ..],..],..]
 *
 * $options_org
 *	 An array to which organism ids are assigned, with name formatted by common name, genus and species or both.
 * 	 Example structure: [org_id: common_name | genus species, ..]
 *
 * $options_map
 * 	 An array to which organism ids are assigned, each holds nested array of maps ids.
 *	 Example structure: $options_map => [13 (organism_id): [225 (map_id) chickpea-JG62xVijay-RIL-2011 (map_name), ..], ..]
 *
 * $options_chr
 *   An array to which organism ids are assigned, each holds a nested array of map ids, and each map id refers to
 *   a nested array of linkage group ids. Each linkage group ids value holds the linkage group name.
 *	 Example structure: $options_chr => [13 (organism_id): [225 (map_id): [1435102 (lg_id): LG1 (lg_name), ..], ..], ..]
 *
 * Notes:
 *	 Results object fetched from chado query structure:
 *	 [0: [map_id: int, map_name: str, linkage_group_id:int, linkage_group: str,
 * 	 	  organism_id: int, genus: str, species: str, common_name: str], ..]
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_select_list_options() {

  // query for the organism drop list selector data records using the feature id of the linkage group
  $sql = "
  SELECT DISTINCT ON (linkage_group_id, organism_id)
  map_id, map_name, linkage_group_id, linkage_group, organism_id, genus, species, common_name
  FROM {tripal_map_genetic_markers_mview} 
  UNION
  SELECT DISTINCT ON (linkage_group_id, organism_id)
  map_id, map_name, linkage_group_id, linkage_group, organism_id, genus, species, common_name
  FROM {tripal_map_qtl_and_mtl_mview}
  ";

  $results = chado_query($sql);
  $positions = array();
  while ($pos = $results->fetchObject()) {
    $positions [] = $pos;
  }
  
  // Obtain drupal variables for options to display organism common name and/or genus and species.
  $common_name = variable_get('tripal_map_toolbar_organism_common_name', FALSE);
  $genus_species = variable_get('tripal_map_toolbar_organism_genus_species', TRUE);

  // obtain the organism, map, and chromosome options for the selector lists, from the query records
  // it is possible for a distinct map to be associated with multiple organisms
  $map_org = array();
  $options_org = array();
  $options_map = array();
  $options_chr = array();
  
  foreach ($positions as $position) {
    $organism_id = $position->organism_id;
    $map_id = $position->map_id;

    if (!array_key_exists($organism_id, $options_org)) {
      // by default, display the genus and species
      $options_org[$organism_id] = $position->genus . " " . $position->species;
      if (($common_name) && !($genus_species)) {
        // display the common name
        $options_org[$organism_id] = $position->common_name;
      }
      elseif (!($common_name) && ($genus_species)) {
        // display the genus and species
        $options_org[$organism_id] = $position->genus . " " . $position->species;
      }
      elseif (($common_name) && ($genus_species)) {
        // display both common name and genus and species
        $options_org[$organism_id] = $position->common_name . " (" . $position->genus . " " . $position->species . ") ";
      }
    }
    // each map can belong to multiple organisms
    if (!array_key_exists($organism_id, $options_map)) {
      $options_map[$organism_id] = array();
    }
    if (!array_key_exists($map_id, $options_map[$organism_id])) {
      $options_map[$organism_id][$map_id] = $position->map_name;
    }
    if (!array_key_exists($organism_id, $options_chr)) {
      $options_chr[$organism_id] = array();
    }
    if (!array_key_exists($map_id, $options_chr[$organism_id])) {
      $options_chr[$organism_id][$map_id] = array();
    }
    if (!array_key_exists($position->linkage_group_id, $options_chr[$organism_id][$map_id])) {
      $options_chr[$organism_id][$map_id][$position->linkage_group_id] = $position->linkage_group;
    }
    if (!array_key_exists($map_id, $map_org)) {
      $map_org[$map_id] = array();
      // when a map is associated with multiple organisms, the first organism is chosen
      $map_org[$map_id]["organism_id"] = $organism_id; // change this to be an array and append on to it 
      $map_org[$map_id]["linkage_groups"] = array();
    }
    if (!array_key_exists($position->linkage_group, $map_org[$map_id]["linkage_groups"])) {
      $map_org[$map_id]["linkage_groups"][$position->linkage_group] = $position->linkage_group_id;
    }
  }

  foreach ($options_chr as $org_id => $maps) {
    // sort the linkage groups by name
    foreach ($maps as $map_id => $lgs) {
      natcasesort($options_chr[$org_id][$map_id]);
    }
    // sort the maps by name
    natcasesort($options_map[$org_id]);
  }
 
  $select_options = array();
  $select_options['map_org'] =& $map_org;
  $select_options['options_org'] =& $options_org;
  $select_options['options_map'] =& $options_map;
  $select_options['options_chr'] =& $options_chr;

  return $select_options;
}


/**
 * function: tripal_map_create_comparison_options
 *	 Take the correspondence data and place it into a format used by the comparison form selector lists.
 *
 * @param $select_options
 *	 correspondences array 	  
 * 
 * @param $featuremap_id
 * 	 the featuremap_id for the reference map	
 * 
 * @param $linkage_group_id
 *   the featuremap_id for the linkage group
 *   
 * @return
 * 	 $options_corres 
 * 	 Array used to populate the comparison form selector lists.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_create_comparison_options($select_options, $featuremap_id, $linkage_group_id) {

  // order the comparison map and chr according to the most correspondences
  $corres = tripal_map_create_correspondences();
  
  // for each map in the correspondences, lookup the organism using map_org, then take the org_id and copy it to org, map and chr.  
  //$map_org => [225 (map_id): [organism_id: 13, linkage_groups [LG1 (lg_name): 1435102( lg_id), ..],..],..]
  $options_corres = array();
  $options_corres['map_org'] = $select_options['map_org'];
  $options_corres['options_org'] = array();
  $options_corres['options_map'] = array();
  $options_corres['options_chr'] = array();
  $options_org =& $options_corres['options_org'];
  $options_map =& $options_corres['options_map'];
  $options_chr =& $options_corres['options_chr'];

  $corres_featuremap = $corres[$featuremap_id];
  // corres_featuremap =>[
  // [432 (map_id): num_corres: 44, linkage_groups: [1631357: 4, lg2: 6, ..]],
  // [428 (map_id): num_corres: 25, linkage_groups: [1631357: 4, lg2: 6, ..]],...]
  foreach ($corres_featuremap as $map_corres_id => $lg_corres) {
    
    // lookup organism associated with map
    $org_id = tripal_map_get_organism_id($select_options['map_org'], $map_corres_id);

    $map_num_corres = 0;
    foreach ($lg_corres as $lg_corres_id => $lgs) {
      foreach ($lgs as $lg_id => $lg_num_corres) {
        if ($lg_id == $linkage_group_id) {
          if (!array_key_exists($org_id, $options_org)) {
            // for options_org, lookup the entry and get the name/species
            // $options_org: [13 (organism_id): common_name | genus species, ..]
            $options_org[$org_id] = $select_options['options_org'][$org_id];
          }
          // maps: add the map_id to the organism array.
          // For each map lookup the map name tripal_map_get_map_name($map_corres_id) also add the num corres for that map
          if (!array_key_exists($org_id, $options_map)) {
            // $options_map => [organism_id 1: [map_id 2:  map_name chickpea-map, ..], ..]
            $options_map[$org_id] = array();
          }

          // chromosomes
          if (!array_key_exists($org_id, $options_chr)) {
            // $options_chr => [13 (organism_id): [225 (map_id): [1435102 (lg_id): LG1 (lg_name), ..], ..], ..]
            $options_chr[$org_id] = array();
          }
          if (!array_key_exists($map_corres_id, $options_chr[$org_id])) {
            $options_chr[$org_id][$map_corres_id] = array();
          }

          // the linkage group for the reference matches the correspondences linkage groups 
          $options_chr[$org_id][$map_corres_id][$lg_corres_id]['num_corres'] = $lg_num_corres;
          $options_chr[$org_id][$map_corres_id][$lg_corres_id]['lg_corres_id'] = $lg_corres_id;
          $options_chr[$org_id][$map_corres_id][$lg_corres_id]['lg_text'] = $select_options['options_chr'][$org_id][$map_corres_id][$lg_corres_id] .
          " (" . $lg_num_corres . ")";
          $map_num_corres += $lg_num_corres;
        }
      }
    }
    if ($map_num_corres > 0) {
      // sort linkage groups in ascending order, according to number of correspondences per linkage group,
      // use uasort to preserve key association
      uasort($options_chr[$org_id][$map_corres_id], 'tripal_map_sort_by_num_corres');

      // assign only the text value to the linkage group
      $tmp_chr_array = $options_chr[$org_id][$map_corres_id];
      foreach ($tmp_chr_array as $lg_id => $lgval) {
        // iterate through using array copy, so array pointer is set correctly to to first elem
        $options_chr[$org_id][$map_corres_id][$lg_id] = $lgval['lg_text'];
      }

      $options_map[$org_id][$map_corres_id]['num_corres'] = $map_num_corres; 
      $options_map[$org_id][$map_corres_id]['map_corres_id'] = $map_corres_id;
      $options_map[$org_id][$map_corres_id]['map_text'] = tripal_map_get_map_name($map_corres_id) . " (" . $map_num_corres . ")";
    }
  }
  
  // lookup organism associated with map
  $featuremap_org_id = tripal_map_get_organism_id($select_options['map_org'], $featuremap_id);
  
  if (count($options_chr[$featuremap_org_id]) > 1) {
    // other maps with correspondences exist, so remove the linkage group that matches the linkage group and map
    // in the reference map to avoid comparing it against itself.
    if (array_key_exists($linkage_group_id, $options_chr[$featuremap_org_id][$featuremap_id])) {
      unset($options_chr[$featuremap_org_id][$featuremap_id][$linkage_group_id]);
    }

    if (count($options_chr[$featuremap_org_id][$featuremap_id] == 0)) {
      // remove the map from the linkage group list	
      unset($options_chr[$featuremap_org_id][$featuremap_id]);
      // remove the map from the list of maps too, since it has no other linkage groups
      unset($options_map[$featuremap_org_id][$featuremap_id]);
    }
  }
  
  foreach ($options_map as $org_id => $maps) {
    // sort the maps in ascending order, according to the number of correspondences per map, use uasort to preserve key association
    uasort($options_map[$org_id], 'tripal_map_sort_by_num_corres');

    $tmp_map_array = $options_map[$org_id]; 
    foreach ($tmp_map_array as $map_id => $mapval) {
      // copy the array of sorted linkage groups, then create a fresh permanent array with only the required values
      // iterate through using array copy, so array pointer is set correctly to to first elem 
      $options_map[$org_id][$map_id] = $mapval['map_text'];
    }
  } 
  return $options_corres;

}


function tripal_map_sort_by_num_corres($b, $a) {
  return $a['num_corres'] - $b['num_corres'];
}

/**
 * function: tripal_map_update_storage
 *
 * @param $form_state
 * @param $key
 * @param $val
 * @param $overwrite
 *
 * @return
 * 	 updates the form_state param
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_update_storage(&$form_state, $key, $val, $overwrite) {

  $fs_storage =& $form_state['storage'];
  // if overwrite is false, only write value if key does not exist
  if (!$overwrite) {
    if (!array_key_exists($key, $fs_storage)) {
      $fs_storage[$key] = $val;
    }
  }
  else {
    $fs_storage[$key] = $val;
  }
}


/**
 * function: tripal_map_get_form_trigger
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @return
 * 	 form state triggering element if it exists, otherwise returns FALSE.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_form_trigger($form_state) {

  $ret = FALSE;
  if (array_key_exists('triggering_element', $form_state)) {
    // An item on the comparison toolbar was modified, or "triggered"
    $ret = $form_state['triggering_element']['#name'];
  }
  return $ret;

}


/**
 * function: tripal_map_get_chr_id
 *
 * @param $map_org
 * @param $featuremap_id
 * @param $linkage_group
 *
 * @return
 * 	 Associated linkage group id for the given feature map id and linkage group name pairing 
 * 
 * @ingroup tripal_map_includes
 */
function tripal_map_get_chr_id($map_org, $featuremap_id, $linkage_group) {

  return $map_org[$featuremap_id]["linkage_groups"][$linkage_group];
}


/**
 * function: tripal_map_get_organism_id
 *
 * @param $map_org
 * @param $featuremap_id
 *
 * @return
 * 	 Associated organism id for the given feature map id
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_organism_id($map_org, $featuremap_id) {

  if (array_key_exists($featuremap_id, $map_org)) {
    return $map_org[$featuremap_id]["organism_id"];
  }
  else {
    return FALSE; 
  }
}


/**
 * function: tripal_map_get_lg_name_from_id
 *
 * @param $form_state 
 * @param $org_id
 * @param $featuremap_id
 * @param $linkage_group_key 
 * 
 * @return
 * 	 Associated linkage group name for the given linkage group id
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_lg_name_from_id($form_state, $org_id, $featuremap_id, $linkage_group_key) {

  $lg_name = $form_state['storage']['chr_all'][$org_id][$featuremap_id][$linkage_group_key];
  return $lg_name;

}


/**
 * function: tripal_map_get_map_name
 *
 * @param $featuremap_id
 *
 * @return
 * 	 Associated map name for the given feature map id
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_map_name($featuremap_id) {

  $map_name = "";
  $results = chado_query("SELECT FM.name FROM {featuremap} FM WHERE FM.featuremap_id = " . $featuremap_id);
  if ($results) {
    $map_name = $results->fetchObject()->name;
  }
  return $map_name;
}

/**
 * function: tripal_get_map_type
 *
 * @param $featuremap_id
 *
 * @return
 * 	 Associated map name for the given feature map id
 *
 * @ingroup tripal_map_includes
 */
function tripal_get_map_type($featuremap_id) {

  $map_type = "";

  $results = chado_query(" SELECT map_type FROM {tripal_map_genetic_markers_mview} MV1 WHERE MV1.map_id = " . $featuremap_id .
      "UNION SELECT map_type FROM {tripal_map_qtl_and_mtl_mview} MV2 WHERE MV2.map_id = " . $featuremap_id);
  if ($results) {
    $map_type = $results->fetchObject()->map_type;
  }

  return $map_type;
}


/**
 * function: tripal_map_get_linkage_groups
 *	 for all views: overview, comparison
 *
 * @param $features
 *
 * @return
 *   The array of linkage groups belonging to all of the features passed in.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_linkage_groups($features) {

  $linkage_groups = array();
  foreach ($features as $feature) {
    if (!in_array($feature['linkage_group'], $linkage_groups)) {
      $linkage_groups[] = $feature['linkage_group'];
    }
  }
  
  return $linkage_groups;
}


/**
 * function: tripal_map_get_marker_types
 *	 for all views: overview, comparison
 *
 * @param $features
 *
 * @return
 *   The array of all marker types belonging to the features passed in.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_get_marker_types($features) {
  $marker_types = array();
  foreach ($features as $feature) {
    foreach ($feature as $key => $val) {
      if ($key === "genetic_marker_type") {
        if (!in_array($val, $marker_types)) {
          $marker_types[$val] = $val;
        }
      }
    }
  }

  return $marker_types;
}


/**
 * function: tripal_map_update_drupal_add_js_params
 *	 Transfer the PHP values for the map_id, map name, linkage group, and features (genetic markers) to Javascript
 *
 * @param $js_setting
 * @param $featuremap_id
 * @param $linkage_group
 * @param $features
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_update_drupal_add_js_params($js_setting, $featuremap_id, $linkage_group, $features) {

  // Add the base url for access in js.
  global $base_url;
  drupal_add_js(array('baseUrl' => $base_url), 'setting');

  if ($js_setting == "mapCorrespondenceMatrixJS") {
  	drupal_add_js(array($js_setting => array('rows' => $features['rows'])), 'setting');
  	drupal_add_js(array($js_setting => array('cols' => $features['cols'])), 'setting');
  	drupal_add_js(array($js_setting => array('correspondences' => $features['correspondences'])), 'setting');
  	 
  	return;
  }
  
  // set the map name
  $map_name = tripal_map_get_map_name($featuremap_id);
  
  if ($js_setting == "mapDotPlotJS" ) {
    drupal_add_js(array($js_setting => array('primary_map' => $map_name)), 'setting');
    drupal_add_js(array($js_setting => array('prim_linkage_group' => $linkage_group)), 'setting');
    drupal_add_js(array($js_setting => array('dotplot_prim_genetic_features' => $features)), 'setting');
  }
  if ($js_setting == "mapDotPlotSecJS" ) {
    drupal_add_js(array($js_setting => array('secondary_map' => $map_name)), 'setting');
    drupal_add_js(array($js_setting => array('secondary_linkage_group' => $linkage_group)), 'setting');
    drupal_add_js(array($js_setting => array('dotplot_sec_genetic_features' => $features)), 'setting');
  }
  
  if ($js_setting == "mapViewerDisplayJS" ) {
    // set the map name
    $my_settings = array('reference_map_name' => $map_name);
    drupal_add_js(array($js_setting => $my_settings), 'setting');

    // set the linkage group
    $my_settings = array('reference_linkage_group' => $linkage_group);
    drupal_add_js(array($js_setting => $my_settings), 'setting');

    // pass features to js
    $my_settings = array('mapViewer_genetic_features' => $features);
    drupal_add_js(array($js_setting => $my_settings), 'setting');
  }
  if ($js_setting == "mapViewerDisplayComparisonJS" ) {
    // set the map name
    drupal_add_js(array($js_setting => array('comparison_map_name' => $map_name)), 'setting');
    // set the linkage group
    drupal_add_js(array($js_setting => array('comparison_linkage_group' => $linkage_group)), 'setting');
    // pass features to js
    $my_settings = array('mapViewer_genetic_features_comparison' => $features);
    drupal_add_js(array($js_setting => $my_settings), 'setting');
  }
  if ($js_setting == "mapOverviewDisplayJS") {
    if (!empty($featuremap_id)) {
      // used by map overview for order the toc
      drupal_add_js(array($js_setting => array('featuremap_id' => $featuremap_id)), 'setting');
    }

    if (!empty($map_name)) {
      drupal_add_js(array($js_setting => array('reference_map_name' => $map_name)), 'setting');

      $map_type = tripal_get_map_type($featuremap_id);
      drupal_add_js(array($js_setting => array('reference_map_type' => $map_type)), 'setting');
 
      // obtain features and pass to js
      // preprocess for MTL, change from heritable_phenotypic_marker <- MTL is specific to mainlab, don't use.
      //$features = preprocess_mtl_featuretype($features);
      drupal_add_js(array($js_setting => array('mapOverview_genetic_features' => $features)), 'setting');
    }
  }

}


function tripal_map_get_elem($arr, $elem) {

  $ret = FALSE;
  if (array_key_exists($elem, $arr)) {
    $ret = $arr[$elem];
  }

  return $ret;
}

/**
 * function: tripal_map_show_map_chr_form_ajax_callback
 *	 Callback for when reference chromosome and/or comparison toolbar are updated
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @return 
 *   A form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_show_map_chr_form_ajax_callback($form, &$form_state) {

  $fs_storage =& $form_state['storage'];

  // reference lists
  $maps =& $fs_storage['maps_all'];
  $chr =& $fs_storage['chr_all'];
  // reference list keys
  $org_key =& $fs_storage['org_key'];
  $maps_key =& $fs_storage['maps_key'];
  $chr_key =& $fs_storage['chr_key'];

  // comparison lists
  $org_comp =& $fs_storage['org_comp'];
  $maps_comp =& $fs_storage['maps_comp'];
  $chr_comp =& $fs_storage['chr_comp'];
  // comparison list keys
  $org_compkey =& $fs_storage['org_compkey'];
  $maps_compkey =& $fs_storage['maps_compkey'];
  $chr_compkey =& $fs_storage['chr_compkey'];
  $show_comp =& $fs_storage['show_comp'];

  // note: form state storage is updated in the form init code, not here. 
  // This is too late for the form currently being loaded
  // to use the new form state storage values.
  
  // =====================Reference=========================
  if ($form_state['triggering_element']['#name'] == 'select_organism') {
    // update reference form lists
    $org_key_t = $form_state['values']['select_organism'];
    $maps_key_t = key($maps[$org_key_t]);
    $chr_key_t = key($chr[$org_key_t][$maps_key_t]);

    $form['organism']['so_frame']['map_chr']['select_map']['#options'] = $maps[$org_key_t];
    $form['organism']['so_frame']['map_chr']['select_map']['#default_value'] = $maps_key_t;
    $form['organism']['so_frame']['map_chr']['select_chromosome']['#options'] = $chr[$org_key_t][$maps_key_t];
    $form['organism']['so_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key_t;

    // get the new comparison list options to those that have correspondences for the reference map and chr
    // now that we have the new comparison list options based on the reference map and chr and reference select options,
    $form = tripal_map_update_comparison_toolbar($form, $form_state, $maps_key_t, $chr_key_t);
  }

  if ($form_state['triggering_element']['#name'] == 'select_map') {
    // update reference map key, and reference chr list and key in the form
    $maps_key_t = $form_state['values']['select_map'];
    $chr_key_t = key($chr[$org_key][$maps_key_t]); 

    $form['organism']['so_frame']['map_chr']['select_map']['#default_value'] = $maps_key_t;
    $form['organism']['so_frame']['map_chr']['select_chromosome']['#options'] = $chr[$org_key][$maps_key_t];
    $form['organism']['so_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key_t;	

    // get the new comparison list options to those that have correspondences for the reference map and chr
    // now that we have the new comparison list options based on the reference map and chr and reference select options,
    $form = tripal_map_update_comparison_toolbar($form, $form_state, $maps_key_t, $chr_key_t);
  }

  //============comparison================
  if ($form_state['triggering_element']['#name'] == 'select_organism_comparison') {

    // take the organism_id value from the form state
    tripal_map_update_storage($form_state, 'org_compkey', $form_state['values']['select_organism_comparison'], TRUE);
    tripal_map_update_storage($form_state, 'maps_comp', $maps_comp[$org_compkey], TRUE);
    tripal_map_update_storage($form_state, 'maps_compkey', key($maps_comp[$org_compkey]), TRUE);
    tripal_map_update_storage($form_state, 'chr_comp', $chr_comp[$org_compkey][$maps_compkey], TRUE);
    tripal_map_update_storage($form_state, 'chr_compkey', key($chr_comp[$org_compkey][$maps_compkey]), TRUE);

    $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#options'] = $fs_storage['maps_comp'];
    $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#default_value'] = $maps_compkey;
    $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#options'] = $fs_storage['chr_comp'];
    $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#default_value'] = $chr_compkey;
  }

  if ($form_state['triggering_element']['#name'] == 'select_map_comparison') {
    // show map or chr
    tripal_map_update_storage($form_state, 'maps_compkey', $form_state['values']['select_map_comparison'], TRUE);
    tripal_map_update_storage($form_state, 'chr_comp', $chr_comp[$org_compkey][$maps_compkey], TRUE);
    tripal_map_update_storage($form_state, 'chr_compkey', key($chr_comp[$org_compkey][$maps_compkey]), TRUE);

    $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#default_value'] = $maps_compkey;
    $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#options'] = $fs_storage['chr_comp'];
    $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#default_value'] = $chr_compkey;
  }
 
  if ($form_state['triggering_element']['#name'] == 'show_comparison') {
    //save the show comparison setting in form state storage, so it is accessible to JS
    tripal_map_update_storage($form_state, 'show_comp', $form_state['values']['show_comparison'], TRUE);
    $form['organism_comparison']['#disabled'] = !$show_comp;

  }
  return $form;

}


/**
 * function: tripal_map_update_comparison_toolbar
 *	 Obtain new comparison list options based on the reference map and chr selections passed in.
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @param $maps_key_t
 * @param $chr_key_t
 * 
 * @return
 *   A form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */

function tripal_map_update_comparison_toolbar($form, $form_state, $maps_key_t, $chr_key_t) {

  $fs_storage =& $form_state['storage'];

  // get the new comparison list options to those that have correspondences for the reference map and chr
  // now that we have the new comparison list options based on the reference map and chr and reference select options,
  $select_options_corres = tripal_map_create_comparison_options($fs_storage['select_options_ref'], $maps_key_t, $chr_key_t);
  $options_org_comp = $select_options_corres['options_org'];
  $options_map_comp = $select_options_corres['options_map'];
  $options_chr_comp = $select_options_corres['options_chr'];
  $org_compkey_t = key($options_org_comp);
  $maps_compkey_t = key($options_map_comp[$org_compkey_t]);
  $chr_compkey_t = key($options_chr_comp[$org_compkey_t][$maps_compkey_t]);

  // update comparison form state lists and keys. Using the reference map and chr key, obtain the form for correspondences options lists
  $form['organism_comparison']['select_organism_comparison']['#options'] = $options_org_comp;
  $form['organism_comparison']['select_organism_comparison']['#default_value'] = $org_compkey_t;
  $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#options'] = $options_map_comp[$org_compkey_t];
  $form['organism_comparison']['map_chr_comparison']['select_map_comparison']['#default_value'] = $maps_compkey_t;
  $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#options'] = $options_chr_comp[$org_compkey_t][$maps_compkey_t];
  $form['organism_comparison']['map_chr_comparison']['select_chromosome_comparison']['#default_value'] = $chr_compkey_t;
  
  return $form;
}


/**
 * function: tripal_map_show_map_chr_landingpage_form_ajax_callback
 *	 Callback for when reference chromosome and/or comparison toolbar are updated
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @return
 *   A form array for the Landing Page form.
 *
 * @ingroup tripal_map_includes
 */

function tripal_map_show_map_chr_landingpage_form_ajax_callback($form, &$form_state) {

	//echo("tripal_map_show_map_chr_landingpage_form_ajax_callback");
	$fs_storage =& $form_state['storage'];

	// reference lists
	$maps =& $fs_storage['maps_all'];
	$chr =& $fs_storage['chr_all'];
	// reference list keys
	$org_key =& $fs_storage['org_key'];
	$maps_key =& $fs_storage['maps_key'];
	$chr_key =& $fs_storage['chr_key'];

	// note: form state storage is updated in the form init code, not here.
	// This is too late for the form currently being loaded
	// to use the new form state storage values.

	// =====================Reference=========================
	if ($form_state['triggering_element']['#name'] == 'select_organism') {
		// update reference form lists
		$org_key_t = $form_state['values']['select_organism'];
		$maps_key_t = key($maps[$org_key_t]);
		$chr_key_t = key($chr[$org_key_t][$maps_key_t]);

		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map']['#options'] = $maps[$org_key_t];
		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map']['#default_value'] = $maps_key_t;
		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#options'] = $chr[$org_key_t][$maps_key_t];
		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key_t;
		
	}

	if ($form_state['triggering_element']['#name'] == 'select_map') {
		
		// update reference map key, and reference chr list and key in the form
		$maps_key_t = $form_state['values']['select_map'];
		$chr_key_t = key($chr[$org_key][$maps_key_t]);

		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_map']['#default_value'] = $maps_key_t;
		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#options'] = $chr[$org_key][$maps_key_t];
		$form['landing_page']['toolbar']['map_select_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key_t;
		
	}
	
	return $form;

}

/**
 * function: tripal_map_show_org_map_form_ajax_callback
 *	 Callback for updating storage for the correspondences matrix form
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @return
 *   A form array for the Correspondences Matrix form.
 *
 * @ingroup tripal_map_includes
 */

function tripal_map_show_org_map_form_ajax_callback($form, &$form_state) {

  $fs_storage =& $form_state['storage'];

  // primary lists and list keys
  $maps =& $fs_storage['maps_all'];
  $cm_prim_org_key = $fs_storage['cm_prim_org_key'];
  $cm_prim_maps_key = $fs_storage['cm_prim_maps_key'];

  // secondary lists and list keys
  $maps_comp =& $fs_storage['maps_all'];
  $cm_sec_org_key = $fs_storage['cm_sec_org_key'];
  $cm_sec_maps_key = $fs_storage['cm_sec_maps_key'];
	
  // note: form state storage is updated in the form init code, not here.
  // This is too late for the form currently being loaded
  // to use the new form state storage values.
	
  // =====================Primary/Reference=========================
  if ($form_state['triggering_element']['#name'] == 'select_organism_primary_cm') {
    // update reference form lists
    $org_key_t = $form_state['values']['select_organism_primary_cm'];
    $maps_key_t = key($maps[$org_key_t]);
    
    // set the map for cm
    $form['correspondence_matrix']['organism_primary']['select_map_primary_cm']['#options'] = $maps[$org_key_t];
    $form['correspondence_matrix']['organism_primary']['select_map_primary_cm']['#default_value'] = $maps_key_t;
  }

  if ($form_state['triggering_element']['#name'] == 'select_map_primary_cm') {
    // update reference map key, and reference chr list and key in the form
    $maps_key_t = $form_state['values']['select_map_primary_cm'];
   
    $form['correspondence_matrix']['organism_primary']['select_map_primary_cm']['#default_value'] = $maps_key_t;
    
  }
	
  //============comparison================
  if ($form_state['triggering_element']['#name'] == 'select_organism_secondary_cm') {
	
	// update reference form lists
	$cm_sec_org_key_t = $form_state['values']['select_organism_secondary_cm'];
	$cm_sec_maps_key_t = key($maps[$cm_sec_org_key_t]);
	
	// set the map for cm
    $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm']['#options'] = $maps[$cm_sec_org_key_t];
    $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm']['#default_value'] = $cm_sec_maps_key_t;
	
  }
	
  if ($form_state['triggering_element']['#name'] == 'select_map_secondary_cm') {
    // show map or chr
  	$cm_sec_maps_key_t = $form_state['values']['select_map_secondary_cm'];
  	 
    $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm']['#default_value'] = $cm_sec_maps_key_t;

  }
	
  return $form;
}


/**
 * function: tripal_map_form_add_show_hide_bar
 *	 Add the toolbar to show or hide rulers and marker positions in the MapViewer form
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @return
 *   The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_add_show_hide_bar(&$form, &$form_state) {

  $form['display_settings']['ds_frame']['show_hide_mapViewer'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'show_hide_fieldset_mapViewer',
      'class' => array("TripalMap")),
      '#collapsible' => FALSE,
  );

  $form['display_settings']['ds_frame']['show_hide_mapViewer']['show_ruler'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show ruler'),
    '#attributes' => array(
      'id' => 'show_ruler_mapViewer'),
      '#default_value' => TRUE,
  );

  $form['display_settings']['ds_frame']['show_hide_mapViewer']['marker_pos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show marker position'),
    '#attributes' => array('id' => 'marker_pos_mapViewer'),
    '#default_value' => FALSE,
  );

  $form['display_settings']['ds_frame']['show_hide_mapViewer']['submit_show_ruler_pos'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
    '#name' => t('Submit3'),
    '#attributes' => array('id' => 'submit_mapViewer_all'),
    '#prefix' => '<div class="submit_mapViewer-all-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_submit_rebuild_form_state'),
    '#after_build' => array('tripal_map_draw_mapViewer'),
  );
  
  return $form;
  
}

