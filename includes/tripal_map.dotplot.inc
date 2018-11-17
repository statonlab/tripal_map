<?php

/**
 * @file
 *	 Create form that displays the Dot Plot
 *	 Draw the Dot Plot page, displaying a plot of correspondences and their relative positions
 *	 on two linkage groups for the maps 
 * 
 * @ingroup tripal_map
 */


/**
 * Implements hook_form().
 *
 * function: tripal_map_dotplot_form
 *   When displaying a map dotplot, we need a form.  This function creates the form 
 *   used for this. Called by URL for a specific route based on the map given parameters
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 *  @param $featuremap_prim_id
 *  @param $lg_prim_id
 *  @param $featuremap_sec_id
 *  @param $lg_sec_id
 *  
 *  @return 
 *   A form array for the correspondences matrix form.
 *    
 * @ingroup tripal_map_includes
 */
function tripal_map_dotplot_form($form, &$form_state, $featuremap_prim_id = NULL, $lg_prim_id = NULL,
		$featuremap_sec_id = NULL,  $lg_sec_id = NULL) {

  // if there is no map supplied do not build the form
  if (!isset($form_state['build_info']['args'][0])) {
    return $form;
  }

  if (((!isset($featuremap_prim_id) || strlen($featuremap_prim_id) == 0)) ||
    ((!isset($lg_prim_id) || strlen($lg_prim_id) == 0)) ||
    ((!isset($featuremap_sec_id) || strlen($featuremap_sec_id) == 0)) ||
  	((!isset($lg_sec_id) || strlen($lg_sec_id) == 0))) {
    return $form;
  }
  
  if (!array_key_exists('storage', $form_state)) {
    $form_state['storage'] = array();
  }
  
  // add the JS files
  tripal_map_draw_dotplot($form, $form_state);
  
  $select_options = tripal_map_get_select_list_options();
  if ((!tripal_map_get_organism_id($select_options['map_org'], $featuremap_prim_id)) ||
     (!tripal_map_get_organism_id($select_options['map_org'], $featuremap_sec_id))) {
    return drupal_not_found();
  }

  // get the linkage group name from the id
  $map_org = $select_options['map_org'];
  $options_chr = $select_options['options_chr'];
  tripal_map_update_storage($form_state, 'chr_all', $options_chr, FALSE);

  $org_prim_id = tripal_map_get_organism_id($map_org, $featuremap_prim_id);
  $lg_prim_name = tripal_map_get_lg_name_from_id($form_state, $org_prim_id, $featuremap_prim_id, $lg_prim_id);
  
  // add the primary map data to JS
  $features = tripal_map_create_features($featuremap_prim_id, $lg_prim_id, $org_prim_id);
  $js_setting = "mapDotPlotJS";
  tripal_map_update_drupal_add_js_params($js_setting, $featuremap_prim_id, $lg_prim_name, $features);

  // add the secondary map data to JS
  $org_sec_id = tripal_map_get_organism_id($map_org, $featuremap_sec_id);
  $lg_sec_name = tripal_map_get_lg_name_from_id($form_state, $org_sec_id, $featuremap_sec_id, $lg_sec_id);

  $features_sec = tripal_map_create_features($featuremap_sec_id, $lg_sec_id, $org_sec_id);
  $js_setting_sec = "mapDotPlotSecJS";
  tripal_map_update_drupal_add_js_params($js_setting_sec, $featuremap_sec_id, $lg_sec_name, $features_sec);
  
  
  // display the dotplot
  $form = tripal_map_attach_d3_lib($form);
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/plotly-latest.min.js';
  
  $form['select'] = array(
    '#type' => 'container',
    '#attributes' => array(
  	  'id' => array('select_fieldset_dotplot'), // JS looks for this id to draw the dotplot
  	  'class' => array("TripalMap")),
  	'#collapsible' => FALSE,
  );
  
  return $form;
}


function tripal_map_draw_dotplot($form, &$form_state) {

  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_map_utils.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/draw_dotplot.js'); 
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_dotplot.js');

  return $form;
}

/**
 * function: tripal_map_submit_cm_rebuild_form_state
 *	 Rebuild the form state, create features from newly selected linkage groups and pass them to JS
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_submit_dp_rebuild_form_state($form, &$form_state) {

  $form_state['rebuild'] = TRUE;
  
}
