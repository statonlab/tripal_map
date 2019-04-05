<?php

/**
 * @file
 *	 Create form that displays correspondence matrix
 *	 Draw the Correspondence Matrix page displaying a table of correspondences between the given
 *	 map, along with toolbars to select display of multi-map correspondences 
 *	 Toolbars select organism, map and linkage group to compare against the selected map 
 * 
 * @ingroup tripal_map
 */


/**
 * Implements hook_form().
 *
 * function: tripal_map_correspondence_matrix_form
 *   When displaying a map correspondences matrix, we need a form.  This function creates the form 
 *   used for this. Called by URL for a specific route based on the map given parameters
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 *  @param $featuremap_prim_id
 *  @param $featuremap_sec_id
 *  
 *  @return 
 *   A form array for the correspondences matrix form.
 *    
 * @ingroup tripal_map_includes
 */
function tripal_map_correspondence_matrix_form($form, &$form_state, $featuremap_prim_id = NULL, $featuremap_sec_id = NULL) {

  // if there is no map supplied do not build the form
  if (!isset($form_state['build_info']['args'][0])) {
    return drupal_not_found();
  }

  if (((!isset($featuremap_prim_id) || strlen($featuremap_prim_id) == 0)) ||
      ((!isset($featuremap_sec_id) || strlen($featuremap_sec_id) == 0))) {
    return drupal_not_found();
  }

  // Generate the organism, and map selector lists data for the correspondence matrix.
  if (!array_key_exists('storage', $form_state)) {
  	$form_state['storage'] = array();
  }
  
  $select_options = tripal_map_get_select_list_options();
  if ((!tripal_map_get_organism_id($select_options['map_org'], $featuremap_prim_id)) ||
  	(!tripal_map_get_organism_id($select_options['map_org'], $featuremap_sec_id))) {
  	  return drupal_not_found();
  }
  
  // add the JS files
  tripal_map_draw_correspondence_matrix($form, $form_state);
  
  /* Enable to show selector menus */
  // Initialize toolbars and form state storage
  // The secondary map will have a filtered list based on the primary map
  if (tripal_map_show_correspondence_matrix_menu()) {
    tripal_map_form_cm_organism_bar_init($form, $form_state, $select_options, $featuremap_prim_id, $featuremap_sec_id);
    tripal_map_form_correspondence_matrix_bar($form, $form_state, $select_options, $featuremap_prim_id, $featuremap_sec_id);
  }
  
  // add refmap, compmap and corres to JS
  $map_org = $select_options['map_org'];
  $features_corres_matrix = tripal_map_create_cm_JS_params($featuremap_prim_id, $featuremap_sec_id, $map_org);
  //$features_corres_matrix = {rows: [type: refmap,  name: $ref_map_name, lgs: [lg1, lg2, ...]}, cols: {type: compmap,  name: $comp_map_name, lgs: [lg1, lg2, ...]}, corresp: [[1,2,...],[2,3,...],...]}
  $js_setting_corres_matrix = 'mapCorrespondenceMatrixJS';
  tripal_map_update_drupal_add_js_params($js_setting_corres_matrix, NULL, NULL, $features_corres_matrix);
  
  // display the correspondence matrix table
  $form = tripal_map_attach_d3_lib($form);
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/FileSaver.js';
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/canvas-to-blob.min.js';
  
  $form['select'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => array('select_fieldset_correspondence_matrix'), // JS looks for this id to draw the matrix
      'class' => array("TripalMap")),
    '#collapsible' => FALSE,
  );
  
  return $form;
}


function tripal_map_create_cm_JS_params($featuremap_prim_id, $featuremap_sec_id, $map_org) {

	// correspondences
  $corres = tripal_map_create_correspondences();
  
  $map_corres_prim = reset($corres[$featuremap_sec_id]); // default to the first prim corres match
  //$map_corres_prim [1632806:[1632917:42, 1632944:5, ..], 1632850:[1632925:37, 1632..], ...]
  $ak_corres_featuremap_sec = array_keys($corres[$featuremap_sec_id]);
  $tmp_featuremap_prim_id = reset($ak_corres_featuremap_sec);
  
  $ref_map_lgs = $map_org[$tmp_featuremap_prim_id]['linkage_groups'];
  //[lg1:234325, lg2:344509, ...]
  if (array_key_exists($featuremap_prim_id, $corres[$featuremap_sec_id])) {
    // if the passed-in prim comparison map has correspondences, assign them.
    $map_corres_prim = $corres[$featuremap_sec_id][$featuremap_prim_id];
    $ref_map_lgs = $map_org[$featuremap_prim_id]['linkage_groups'];
  }
  else {
  	// the prim map does not have correspondences, assign it to the default
  	$featuremap_prim_id = $tmp_featuremap_prim_id;
  }
  $comp_map_lgs = $map_org[$featuremap_sec_id]['linkage_groups'];
  ksort($ref_map_lgs);
  ksort($comp_map_lgs);
  
  $features_corres_matrix = array();
  // reference map
  $features_corres_matrix['rows'] = array();
  $features_cm_rows = &$features_corres_matrix['rows'];
  $features_cm_rows['type'] = 'refmap';
  $features_cm_rows['name'] = tripal_map_get_map_name($featuremap_prim_id);
  $features_cm_rows['feature_id'] = $featuremap_prim_id;
  $features_cm_rows['lgs'] = array_keys($ref_map_lgs);
  
  // comparison map
  $features_corres_matrix['cols'] = array();
  $features_cm_cols = &$features_corres_matrix['cols'];
  $features_cm_cols['type'] = 'compmap';
  $features_cm_cols['name'] = tripal_map_get_map_name($featuremap_sec_id);
  $features_cm_cols['feature_id'] = $featuremap_sec_id;
  $features_cm_cols['lgs'] = array_keys($comp_map_lgs);
  
  $features_corres_matrix['correspondences'] = array();
  foreach ($ref_map_lgs as $ref_map_lg_name => $ref_map_lg_id) {
	$features_corres_matrix['correspondences'][$ref_map_lg_id] = array();
	if (array_key_exists($ref_map_lg_id, $map_corres_prim)) {
	  $lg_corres_sec = $map_corres_prim[$ref_map_lg_id];
	  foreach ($comp_map_lgs as $comp_map_lg_name => $comp_map_lg_id) {
	    if (array_key_exists($comp_map_lg_id, $lg_corres_sec)) {
		  $lg_corres_sec_val = $lg_corres_sec[$comp_map_lg_id];
		  // append the number of correspondences between the two linkage groups to the array
		  array_push($features_corres_matrix['correspondences'][$ref_map_lg_id], $lg_corres_sec_val);
		}
		else {
		  array_push($features_corres_matrix['correspondences'][$ref_map_lg_id], 0);
		}
	  }
	}
	else {
		$ref_map_lg_id_corres = array_fill(0, sizeof($comp_map_lgs), 0);
		$features_corres_matrix['correspondences'][$ref_map_lg_id] = $ref_map_lg_id_corres;
	}
  }
  $features_corres_matrix['correspondences'] = array_values($features_corres_matrix['correspondences']);

  return $features_corres_matrix;
}


/**
 * function: tripal_map_form_cm_organism_bar_init
 *	 Initialize the reference toolbar, using trigger values if available.
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 *  @param $featuremap_id
 *  @param $linkage_group
 *  @param $features
 *
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
// assigns the $features array based on the featuremap id
function tripal_map_form_cm_organism_bar_init( &$form, &$form_state, $select_options, &$featuremap_cm_prim_id, &$featuremap_cm_sec_id) {
 
  if (array_key_exists('storage', $form_state)) {
    // the form was updated, but the url is not up to date yet, obtain previous value from form state storage
    // use the linkage group and map values from the form state
    if (array_key_exists('featuremap_cm_prim_id', $form_state['storage'])) {
      $featuremap_cm_prim_id = $form_state['storage']['featuremap_cm_prim_id'];
    }
    if (array_key_exists('featuremap_cm_sec_id', $form_state['storage'])) {
      $featuremap_cm_sec_id = $form_state['storage']['featuremap_cm_sec_id'];
    }
  }

  tripal_map_init_cm_organism_selectors_from_storage($form_state, $select_options, $featuremap_cm_prim_id, $featuremap_cm_sec_id);
  
  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];
  $fs_storage =& $form_state['storage'];

  // primary
  $selector_toolbar = "correspondence_matrix_primary";
  $fs_cmprim_orgkey_val = $fs_storage['cm_prim_org_key'];
  $fs_cmprim_mapskey_val = $fs_storage['cm_prim_maps_key'];

  tripal_map_update_storage($form_state, 'cm_prim_maps', $options_map[$fs_cmprim_orgkey_val], FALSE);

  $fs_selector_names = array("orgkey" => "cm_prim_org_key", "maps" => "cm_prim_maps", 
    "mapskey" => "cm_prim_maps_key", "chr" => "", "chrkey" => "");
  $form_trigger_selectors = array(  "select_organism" => "select_organism_primary_cm", 
    "select_map" => "select_map_primary_cm", "select_chromosome" =>"");
  
  tripal_map_trigger_update_storage_from_selectors($selector_toolbar, $fs_selector_names,
    $form_trigger_selectors, $select_options, $form_state);

  $form['correspondence_matrix']['organism_primary']['select_organism_primary_cm']['#default_value'] = $fs_cmprim_orgkey_val;
  $form['correspondence_matrix']['organism_primary']['select_map_primary_cm']['#options'] = $fs_storage['cm_prim_maps'];
  $form['correspondence_matrix']['organism_primary']['select_map_primary_cm']['#default_value'] = $fs_cmprim_mapskey_val;

  // secondary
  $selector_toolbar = "correspondence_matrix_secondary";
  $fs_cmsec_orgkey_val = $fs_storage['cm_sec_org_key'];
  $fs_cmsec_mapskey_val = $fs_storage['cm_sec_maps_key'];

  tripal_map_update_storage($form_state, 'cm_sec_maps', $options_map[$fs_cmsec_orgkey_val], FALSE);

  $fs_selector_names = array("orgkey" => "cm_prim_sec_key", "maps" => "cm_sec_maps",
    "mapskey" => "cm_sec_maps_key", "chr" => "", "chrkey" => "");
  $form_trigger_selectors = array(  "select_organism" => "select_organism_secondary_cm",
    "select_map" => "select_map_secondary_cm", "select_chromosome" =>"");
  tripal_map_trigger_update_storage_from_selectors($selector_toolbar, $fs_selector_names,
    $form_trigger_selectors, $select_options, $form_state);

  $form['correspondence_matrix']['organism_secondary']['select_organism_secondary_cm']['#default_value'] = $fs_cmsec_orgkey_val;
  $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm']['#options'] = $fs_storage['cm_sec_maps'];
  $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm']['#default_value'] = $fs_cmsec_mapskey_val;

  tripal_map_update_storage($form_state, 'featuremap_cm_prim_id', $fs_cmprim_mapskey_val, TRUE);
  tripal_map_update_storage($form_state, 'featuremap_cm_sec_id', $fs_cmsec_mapskey_val, TRUE);

  return $form;

}


/**
 *
 * function: tripal_map_init_cm_organism_selectors_from_storage
 *	 Initialize form state storage variables for selectors
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $featuremap_id
 * @param $linkage_group
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_init_cm_organism_selectors_from_storage(&$form_state, $select_options,  $featuremap_prim_id, $featuremap_sec_id) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];

  $fs_storage =& $form_state['storage'];
  if (!(array_key_exists('storage', $form_state))) {
    $fs_storage = array();
  }
  if (!is_array($fs_storage)) {
    // storage is initialized by Drupal to null, set it to type array
    $fs_storage = array();
  }

  //cache the select options arrays for org, map 
  tripal_map_update_storage($form_state, 'select_options_ref', $select_options, FALSE);
  tripal_map_update_storage($form_state, 'organisms_all', $options_org, FALSE);
  tripal_map_update_storage($form_state, 'maps_all', $options_map, FALSE);
  $fs_org_key_name = 'cm_prim_org_key';
  $fs_map_key_name = 'cm_prim_maps_key';
  tripal_map_set_select_options_keys_in_storage($form_state, $select_options, 
  		$featuremap_prim_id, $fs_org_key_name, $fs_map_key_name);
  $fs_org_key_name = 'cm_sec_org_key';
  $fs_map_key_name = 'cm_sec_maps_key';
  tripal_map_set_select_options_keys_in_storage($form_state, $select_options, 
  		$featuremap_sec_id, $fs_org_key_name, $fs_map_key_name);
  
}


function tripal_map_set_select_options_keys_in_storage(&$form_state, $select_options, $featuremap_id,
		$fs_org_key_name, $fs_map_key_name) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];

  $fs_storage =& $form_state['storage'];

  // set the keys for the select options arrays for org, map
  if (!(array_key_exists($fs_org_key_name, $fs_storage))) {
    // take the first organism as default
    tripal_map_update_storage($form_state, $fs_org_key_name, key($options_org), FALSE);
    if ((isset($featuremap_id) && strlen($featuremap_id) > 0)) {
      // if featuremap id is available, lookup the associated organism and use that as default
      tripal_map_update_storage($form_state, $fs_org_key_name, tripal_map_get_organism_id($map_org, $featuremap_id), TRUE);
    }
  }
  
  if (!(array_key_exists($fs_map_key_name, $fs_storage))) {
    // use the first map as default
    tripal_map_update_storage($form_state, $fs_map_key_name, key($options_map[$fs_storage[$fs_org_key_name]]), FALSE);
    if (isset($featuremap_id) && strlen($featuremap_id)) {
      // set the default map to the id passed by featuremap_id
      tripal_map_update_storage($form_state, $fs_map_key_name, $featuremap_id, TRUE);
    }
  }
  
}


/**
 * function: tripal_map_form_correspondence_matrix_bar
 *	 Create the reference toolbar with selectors for organism, map and chromosome
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $select_options
 * 
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_correspondence_matrix_bar(&$form, &$form_state, $select_options, $featuremap_cm_prim_id, $featuremap_cm_sec_id) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];

  $fs_storage =& $form_state['storage'];
  $cm_prim_org_key = $fs_storage['cm_prim_org_key'];
  $cm_prim_maps_key = $fs_storage['cm_prim_maps_key'];
  $cm_sec_org_key = $fs_storage['cm_sec_org_key'];
  $cm_sec_maps_key = $fs_storage['cm_sec_maps_key'];
  
  $form['correspondence_matrix'] = array(
    '#type' => 'container',
  	'#prefix' => '<div id="select_correspondence_matrix" class="TripalMap">',
  	'#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
  
  $form['correspondence_matrix']['organism_primary'] = array(
    '#type' => 'container',
  	'#prefix' => '<div id="select_organism_primary_cm" class="TripalMap">',
  	'#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
   
  $form['correspondence_matrix']['organism_primary']['select_organism_primary_cm'] = array(
    '#type' => 'select',
    '#title' => t('Species'),
  	'#prefix' => '<div id="select_organism_primary_cm" class="form-item">',
  	'#suffix' => '</div>',
  	'#ajax' => array(
      'wrapper' => 'show-org-correspondence_matrix-form',
      'callback' => 'tripal_map_show_org_map_form_ajax_callback',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
      '#options' => $options_org,
      '#multiple' => FALSE,
      '#default_value' => $cm_prim_org_key,
  );
  
  $form['correspondence_matrix']['organism_primary']['select_map_primary_cm'] = array(
    '#type' => 'select',
    '#title' => t('Map'),
    '#prefix' => '<div id="select_map_primary_cm" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-org-correspondence_matrix-form',
      'callback' => 'tripal_map_show_org_map_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => $fs_storage['cm_prim_maps'],
    '#multiple' => FALSE,
    '#default_value' => $cm_prim_maps_key,
    '#needs_validation' => FALSE,
  );

  $form['correspondence_matrix']['organism_secondary'] = array(
  		'#type' => 'container',
  		'#prefix' => '<div id="select_organism_secondary_cm" class="TripalMap"><label class="reference">Compare To</label>',
  		'#suffix' => '</div>',
  		'#collapsible' => FALSE,
  );

  $form['correspondence_matrix']['organism_secondary']['select_organism_secondary_cm'] = array(
     '#type' => 'select',
     '#title' => t('Species'),
  	  '#prefix' => '<div id="select_organism_secondary_cm" class="form-item">',
  	  '#suffix' => '</div>',
     '#ajax' => array(
       'wrapper' => 'show-org-correspondence_matrix-form',
       'callback' => 'tripal_map_show_org_map_form_ajax_callback',
        'method' => 'replace',
  		'progress' => array('type' => 'throbber'),
  	  ),
     '#options' => $options_org,
     '#multiple' => FALSE,
     '#default_value' => $cm_sec_org_key,
  );
  
  $form['correspondence_matrix']['organism_secondary']['select_map_secondary_cm'] = array(
    '#type' => 'select',
    '#title' => t('Map'),
    '#prefix' => '<div id="select_map_secondary_cm" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-org-correspondence_matrix-form',
      'callback' => 'tripal_map_show_org_map_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
  	  'method' => 'replace',
     ),
     '#options' => $fs_storage['cm_sec_maps'],
     '#multiple' => FALSE,
     '#default_value' => $cm_sec_maps_key,
     '#needs_validation' => FALSE,
  );
  
  $form['correspondence_matrix']['organism_secondary']['button_cm-submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
    '#name' => ('Submit_CM'),
    '#attributes' => array(
      'id' => 'cm-submit'
    ),
    '#prefix' => '<div class="cm-submit-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_submit_cm_rebuild_form_state'),
    '#after_build' => array('tripal_map_draw_correspondence_matrix'),
  );

  $form['#prefix'] = '<div id="show-org-correspondence_matrix-form">';
  $form['#suffix'] = '</div>';

  return $form;

}


function tripal_map_draw_correspondence_matrix($form, &$form_state) {

  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_map_utils.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/draw_correspondence_matrix.js'); 
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_correspondence_matrix.js');
  drupal_add_js(array( 'tripal_map' => array('modulePath' => drupal_get_path( 'module', 'tripal_map' )) ),'setting');
  
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
function tripal_map_submit_cm_rebuild_form_state($form, &$form_state) {

  $form_state['rebuild'] = TRUE;
  
}

