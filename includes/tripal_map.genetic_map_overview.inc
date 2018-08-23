<?php

/**
 * @file
 *	 Create form that displays genetic maps
 *   Draws the map chromosome glyphs at the top of the Tripal "Map Overview" tab.
 *	 Clicking on a glyph launches the MapViewer page displaying the corresponding chromosome.
 *
 * @ingroup tripal_map
 */


/**
 * function: tripal_map_genetic_map_overview_form
 *	 Initialize the Map Overview form 
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $featuremap_id
 * 	 The map id for the map to be drawn
 *
 * @return
 *   The form array for the Map Overview form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_genetic_map_overview_form($form, &$form_state, $featuremap_id) {

  // if there is no map supplied don't build the form
  if (!isset($form_state['build_info']['args'][0])) {
    return $form;
  }

  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_map_utils.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/draw_genetic_map_overview.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_genetic_map_overview.js');
  //$form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/d3.v4.min.js';
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/d3v3.min.js';
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/FileSaver.js';
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/canvas-to-blob.min.js';
  
  
  // create data structure of format: {{['linkage group']['genetic_marker_locus_name']['marker_pos']},...}
  $features = tripal_map_create_features($featuremap_id);
  $map_name = tripal_map_get_map_name($featuremap_id);

  if (!array_key_exists('storage', $form_state)) {
    $form_state['storage'] = array();
  }

  tripal_map_update_storage($form_state, 'flag_update_js_color', TRUE, FALSE);
  tripal_map_update_storage($form_state, 'flag_update_js_state', TRUE, FALSE);
    
  $linkage_group = ''; 
  $featuremap_id_comparison = '';
  $linkage_group_comparison = '';
  $features_reference = $features; 
  $features_comparison = array(); 
  $marker_types = array(); 
  $color_map = array();
  $form_type = "mapOverview";
  $form = tripal_map_form_init($form, $form_state, $form_type, $featuremap_id, $linkage_group, $featuremap_id_comparison, $linkage_group_comparison, $features_reference, $features_comparison, $marker_types, $color_map);
  
  $form['select'] = array(
    '#type' => 'container',
    '#attributes' => array(
     '#collapsible' => FALSE,)
  );

  // Do not display maps that are in the list of excluded maps
  $exclude_maps = explode(";", variable_get('tripal_map_map_overview_exclude_maps'));
  $exclude_maps = array_map('trim', $exclude_maps);
  $map_name = tripal_map_get_map_name($featuremap_id);
  $b  = in_array($map_name, $exclude_maps);
  
  if (!in_array($map_name, $exclude_maps)) {
    
    $form['select']['MapViewer_textinfo'] = array(
      '#markup' => '<p id="mapoverview-textinfo">Click on any linkage group to explore this map further.
      You will be taken to MapViewer and leave this page.</p>'
    );

    $form['select']['MapViewer'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'id' => array('select_fieldset_genetic_map_overview'),
        'class' => array("TripalMap")),
      '#collapsible' => FALSE,
    );
  }
  return $form;
}

