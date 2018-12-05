<?php

/**
 * @file
 *	 Create form that displays genetic maps
 *	 Draw the MapViewer page displaying linkage group with zoomed view and toolbars below.
 *	 Toolbars select organism, map and linkage group and configure color and marker visibility.
 * 
 * @ingroup tripal_map
 */


/**
 * Implements hook_form().
 *
 * function: tripal_map_mapviewer_form
 *   When displaying a genetic map we need a form.  This function creates the form 
 *   used for this. Called by preprocess hook from a featuremap node or by URL 
 *   for a specific route based on the given map and linkage group parameters
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 *  @param $comparison_type
 *  
 *  @param $args
 *    
 * @return 
 *   A form array for the MapViewer form.
 *    
 * @ingroup tripal_map_includes
 */
function tripal_map_mapviewer_form($form, &$form_state, $comparison_type, $args ) {

    
    // if there is no map supplied do not build the form
    if (!isset($form_state['build_info']['args'][0])) {
        return $form;
    }

    $featuremap_id = $args["featuremap_id"];
    $linkage_group = $args["linkage_group"];
    $genetic_marker_name = "";
    
    if (!isset($featuremap_id) || strlen($featuremap_id) == 0) {
        return $form;
    }
    
    $select_options = tripal_map_get_select_list_options();
    if (!tripal_map_get_organism_id($select_options['map_org'], $featuremap_id)) {
        // the requested map does not exist
        return drupal_not_found();
    }
    
    if (!array_key_exists('storage', $form_state)) {
        $form_state['storage'] = array();
    }
    
    $fs_storage =& $form_state['storage'];
    if (!is_array($fs_storage)) {
        // the form state is initialized to null, by Drupal. Assign it to type array.
        $fs_storage = array();
    }

    
    if (($comparison_type == TRUE) && (!$form_state['rebuild']))  {
      $featuremap_id_comp = $args["featuremap_comp_id"];
      $linkage_group_comp = $args["linkage_group_comp"];

      // get the organism key from the map and linkage group
      // Use the reference map and lg to filter the comparison map and lg lists (adding in text for number of correspondences).
      $linkage_group_id = tripal_map_get_chr_id($select_options['map_org'], $featuremap_id, $linkage_group);
      $select_options_comparison = tripal_map_create_comparison_options($select_options, $featuremap_id, $linkage_group_id);
      $map_org_comp = $select_options_comparison['map_org'];
      
      $fs_storage['org_compkey'] = tripal_map_get_organism_id($map_org_comp, $featuremap_id_comp);
      $fs_storage['maps_compkey'] = $featuremap_id_comp;
      $linkage_group_comp_id = tripal_map_get_chr_id($select_options['map_org'], $featuremap_id_comp, $linkage_group_comp);
      $fs_storage['chr_compkey'] = $linkage_group_comp_id;
      $fs_storage['show_comp'] = TRUE;
    }
    else {
      $genetic_marker_name = $args["genetic_marker_name"];
    }
    
    tripal_map_set_form_user_instr($form);
    
    // set the color and state flags to false, if they do not exist yet.
    tripal_map_update_storage($form_state, 'flag_update_js_color', FALSE, FALSE);
    tripal_map_update_storage($form_state, 'flag_update_js_state', FALSE, FALSE);
    
    $features = array();
    tripal_map_form_organism_bar_init($form, $form_state, $select_options, $featuremap_id, $linkage_group, $features);
    tripal_map_form_add_organism_bar($form, $form_state, $select_options);

    // Generate the organism, map and chromosome selector lists data for the reference map.
    // Initialize toolbars and form state storage
    // The comparison map will have a filtered list based on the chosen map and linkage group
    // Use the reference map and lg to filter the comparison map and lg lists (adding in text for number of correspondences).
    $linkage_group_id = tripal_map_get_chr_id($select_options['map_org'], $featuremap_id, $linkage_group);
    $select_options_comparison = tripal_map_create_comparison_options($select_options, $featuremap_id, $linkage_group_id);
    
    // order the maps from largest number of correspondences to least
    // assigns features_comparison
    $features_comparison = array();
    $show_comparison = FALSE;
    $featuremap_id_comparison = "";
    $linkage_group_comparison = "";
    tripal_map_form_organism_comparison_bar_init($form, $form_state, $select_options_comparison, $featuremap_id_comparison,
        $linkage_group_comparison, $features_comparison, $show_comparison);
    tripal_map_form_add_organism_comparison_bar($form, $form_state, $select_options_comparison);
    
    $form_type = "mapViewer";
    tripal_map_form_init($form, $form_state, $form_type, $featuremap_id, $linkage_group, $featuremap_id_comparison,
        $linkage_group_comparison, $features, $features_comparison, $marker_types, $color_map);
    
    tripal_map_form_add_toolbars( $form, $form_state, $marker_types, $color_map);
    
    return $form;
}



function tripal_map_set_form_user_instr(&$form) {

  // Set the user instructions at the top of the MapViewer form page, with using the drupal variables set
  // in the admin page
  $form['user_instructions'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => array('user_instructions'), // the js looks for this id to draw the map
      'class' => array("TripalMap")),
      '#collapsible' => FALSE,
  );

  $form['user_instructions']['change_map'] = array(
    '#markup' => '<div>Change Map</div>',
    '#prefix' => t('<div  id="change_map_anchor_href"><a href= "#map_anchor">'),
    '#suffix' => t('</a></div>'),
  );

  $video_tut_link = variable_get('tripal_map_video_tutorial_link', t(''));
  $form['user_instructions']['tutorial'] = array(
    '#markup' => '<div id="tutorial"><a href="' . $video_tut_link . '" target="_blank">Short video tutorial</a></div>',
  );

  // obtain link to text tutorial
  $text_tut_link = variable_get('tripal_map_text_tutorial_link', t(''));
  global $base_url;
  if (substr($text_tut_link, 0, 4) != "http") {
      // the sample map path is not a fully formed path ex. "https://.."
      // to prevent appending of incorrect base path because of hook form_alter, set the full path here
      $text_tut_link = $base_url . $text_tut_link;
  }
  
  $form['user_instructions']['text_tutorial'] = array(
    '#markup' => '<div id="text_tutorial">| <a href="' . $text_tut_link . '" target="_blank"> Text tutorial</a></div>',
  );

  $form['select'] = array(
  '#type' => 'container',
    '#attributes' => array(
    'id' => array('select_fieldset_mapViewer'), // JS looks for this id to draw the map
      'class' => array("TripalMap")),
      '#collapsible' => FALSE,
  );
  
}


/**
 *
 * function: tripal_map_init_organism_selectors_from_storage
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
function tripal_map_init_organism_selectors_from_storage(&$form_state, $select_options, $featuremap_id, $linkage_group) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];

  $fs_storage =& $form_state['storage'];
  if (!(array_key_exists('storage', $form_state))) {
    $fs_storage = array();
  }

  if (!is_array($fs_storage)) {
    // storage is initialized by Drupal to null, set it to type array
    $fs_storage = array();
  }

  // cache the select options arrays for org, map and chr
  tripal_map_update_storage($form_state, 'select_options_ref', $select_options, FALSE);
  tripal_map_update_storage($form_state, 'organisms_all', $options_org, FALSE);
  if (!(array_key_exists('maps_all', $fs_storage))) {
    tripal_map_update_storage($form_state, 'maps_all', $options_map, FALSE);
    // the map storage is not created yet, so set the flag to update these values in JS and use on redraw
    if (array_key_exists('flag_update_js_color', $form_state['storage'])) {
      $form_state['storage']['flag_update_js_color'] = TRUE;
    }
    if (array_key_exists('flag_update_js_state', $form_state['storage'])) {
      $form_state['storage']['flag_update_js_state'] = TRUE;
    }
  }
  tripal_map_update_storage($form_state, 'chr_all', $options_chr, FALSE);

  // set the keys for the select options arrays for org, map and chr
  if (!(array_key_exists('org_key', $fs_storage))) {
    // take the first organism as default
    tripal_map_update_storage($form_state, 'org_key', key($options_org), FALSE);
    if ((isset($featuremap_id) && strlen($featuremap_id) > 0)) {
      // if featuremap id is available, lookup the associated organism and use that as default
      tripal_map_update_storage($form_state, 'org_key', tripal_map_get_organism_id($map_org, $featuremap_id), TRUE);
    }
  }
  if (!(array_key_exists('maps_key', $fs_storage))) {
    // use the first map as default
    tripal_map_update_storage($form_state, 'maps_key', key($options_map[$fs_storage['org_key']]), FALSE);
    if (isset($featuremap_id) && strlen($featuremap_id)) {
      // set the default map to the id passed by featuremap_id
      tripal_map_update_storage($form_state, 'maps_key', $featuremap_id, TRUE);
    }
  }
  if (!(array_key_exists('chr_key', $fs_storage))) {
    // use the first chromosome as a default
    tripal_map_update_storage($form_state, 'chr_key', key($options_chr[$fs_storage['org_key']][$fs_storage['maps_key']]), FALSE);
    if ((isset($featuremap_id) && strlen($featuremap_id) > 0 ) &&
        (isset($linkage_group) && strlen($linkage_group) > 0 )) {
      // set the default linkage group to the id passed by linkage group id
      tripal_map_update_storage($form_state, 'chr_key', tripal_map_get_chr_id($map_org, $featuremap_id, $linkage_group), TRUE);
    }
  }
}


/**
 * function: tripal_map_form_organism_bar_init
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
function tripal_map_form_organism_bar_init( &$form, &$form_state, $select_options, &$featuremap_id, &$linkage_group, &$features) {

  if (array_key_exists('storage', $form_state)) {
    // the form was updated, but the url is not up to date yet, obtain previous value from form state storage
    // use the linkage group and map values from the form state
    if (array_key_exists('featuremap_id', $form_state['storage'])) {
      $featuremap_id = $form_state['storage']['featuremap_id'];
    }
    if (array_key_exists('linkage_group', $form_state['storage'])) {
      $linkage_group = $form_state['storage']['linkage_group'];
    }
  }

  tripal_map_init_organism_selectors_from_storage($form_state, $select_options, $featuremap_id, $linkage_group);
  
  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];
  
  $fs_storage =& $form_state['storage'];
  $org_key =& $fs_storage['org_key'];
  $maps_key =& $fs_storage['maps_key'];
  $chr_key =& $fs_storage['chr_key'];

  tripal_map_update_storage($form_state, 'maps', $options_map[$org_key], FALSE);
  tripal_map_update_storage($form_state, 'chr', $options_chr[$org_key][$maps_key], FALSE);

  $selector_toolbar = "mapviewer_reference";
  $fs_selector_names = array("orgkey" => "org_key", "maps" => "maps", "mapskey" => "maps_key", 
  		"chr" => "chr", "chrkey" => "chr_key");
  $form_trigger_selectors = array(  "select_organism" => "select_organism", "select_map" => "select_map", 
  		"select_chromosome" =>"select_chromosome");
  
  tripal_map_trigger_update_storage_from_selectors($selector_toolbar, $fs_selector_names, 
  		$form_trigger_selectors, $select_options, $form_state);

  // Add the dot plot container first so it appears above the control panel
  $form['dotplot'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="show_dot_plot" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE
  );
  
  $form['organism']['so_frame']['select_organism']['#default_value'] = $org_key;
  $form['organism']['so_frame']['map_chr']['select_map']['#options'] = $fs_storage['maps'];
  $form['organism']['so_frame']['map_chr']['select_map']['#default_value'] = $maps_key;
  $form['organism']['so_frame']['map_chr']['select_chromosome']['#options'] = $fs_storage['chr'];
  $form['organism']['so_frame']['map_chr']['select_chromosome']['#default_value'] = $chr_key;

  tripal_map_update_storage($form_state, 'linkage_group', $fs_storage['chr'][$chr_key], TRUE);
  $linkage_group = $fs_storage['linkage_group'];
  tripal_map_update_storage($form_state, 'featuremap_id', $maps_key, TRUE);

  // get marker data based on featuremap_id
  $featuremap_id = $maps_key;
  $organism_id = $org_key;
  $linkage_group_id = $chr_key;
  $features = tripal_map_create_features($featuremap_id, $linkage_group_id, $organism_id);
  
  return $form;

}


function tripal_map_trigger_update_storage_from_selectors($selector_toolbar,
  $fs_selector_names, $form_trigger_selectors, $select_options, &$form_state) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];

  $form_organism_trigger_name = $form_trigger_selectors['select_organism'];
  $form_map_trigger_name = $form_trigger_selectors['select_map'];
  $form_chromosome_trigger_name = $form_trigger_selectors['select_chromosome'];

  $fs_orgkey_name = $fs_selector_names['orgkey'];
  $fs_maps_name = $fs_selector_names['maps'];
  $fs_mapskey_name = $fs_selector_names['mapskey'];
  $fs_chr_name = $fs_selector_names['chr'];
  $fs_chrkey_name = $fs_selector_names['chrkey'];

  $fs_storage =& $form_state['storage'];
  $fs_orgkey_val =& $fs_storage[$fs_orgkey_name];
  $fs_mapskey_val =& $fs_storage[$fs_mapskey_name];
  $fs_chrkey_val =& $fs_storage[$fs_chrkey_name];
  $trigger = tripal_map_get_form_trigger($form_state);
  
  if (!isset($trigger) || (strlen($trigger) == 0) ) {
      return;
  }
  if ($trigger == $form_organism_trigger_name) {
    tripal_map_update_storage($form_state, $fs_orgkey_name, $form_state['triggering_element']['#value'], TRUE);
    tripal_map_update_storage($form_state, $fs_maps_name, $options_map[$fs_orgkey_val], TRUE);
    tripal_map_update_storage($form_state, $fs_mapskey_name, key($options_map[$fs_orgkey_val]), TRUE);
    if (($selector_toolbar == "mapviewer_reference") || ($selector_toolbar == "landingpage_map_bar")) {
      tripal_map_update_storage($form_state, $fs_chr_name, $options_chr[$fs_orgkey_val][$fs_mapskey_val], TRUE);
      tripal_map_update_storage($form_state, $fs_chrkey_name, key($options_chr[$fs_orgkey_val][$fs_mapskey_val]), TRUE);
    }
  }
  if ($trigger == $form_map_trigger_name) {
    tripal_map_update_storage($form_state, $fs_mapskey_name, $form_state['triggering_element']['#value'], TRUE);
      if (($selector_toolbar == "mapviewer_reference") || ($selector_toolbar == "landingpage_map_bar")) {
      tripal_map_update_storage($form_state, $fs_chr_name, $options_chr[$fs_orgkey_val][$fs_mapskey_val], TRUE);
      tripal_map_update_storage($form_state, $fs_chrkey_name, key($options_chr[$fs_orgkey_val][$fs_mapskey_val]), TRUE);
    }
  }
  if ($trigger == $form_chromosome_trigger_name) {
       tripal_map_update_storage($form_state, $fs_chrkey_name, $form_state['triggering_element']['#value'], TRUE);
  }

}


/**
 * function: tripal_map_form_init
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @param $form_type 
 * @param $featuremap_id
 * @param $linkage_group 
 * @param $featuremap_id_comparison 
 * @param $linkage_group_comparison 
 * @param $features_reference 
 * @param $features_comparison 
 * @param $marker_types 
 * @param $color_map
 *
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_init(&$form, &$form_state, $form_type, $featuremap_id, $linkage_group, 
        $featuremap_id_comparison, $linkage_group_comparison, $features_reference, $features_comparison, 
        &$marker_types, &$color_map) {

  $all_features = array_merge($features_reference, $features_comparison);
  // obtain all marker types
  $marker_types = tripal_map_get_marker_types($all_features);
  
  $form = tripal_map_attach_d3_lib($form);
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/FileSaver.js';
  $form['#attached']['js'][] = drupal_get_path('module', 'tripal_map') . '/theme/js/libraries/canvas-to-blob.min.js';
  
  $js_setting = '';
  if ($form_type === "mapOverview") {

    $js_setting = 'mapOverviewDisplayJS';
    tripal_map_update_drupal_add_js_params($js_setting, $featuremap_id, $linkage_group, $all_features);

    // set the color and state flags to true if they do not already exist
    tripal_map_update_storage($form_state, 'flag_update_js_color', TRUE, FALSE);
    tripal_map_update_storage($form_state, 'flag_update_js_state', TRUE, FALSE);
    
    // assign the form state color to form_state storage and js
    $form_state = tripal_map_assign_form_state_marker_color( $js_setting, $marker_types, $form_state, $color_map);
    $form_state = tripal_map_assign_form_state_marker_type_state( $js_setting, $marker_types, $form_state);

    return $form;
  }

  $flag_update_js_storage = FALSE;
  if ($form_state['storage']['flag_update_js_color'] || $form_state['storage']['flag_update_js_state']) {
    $flag_update_js_storage = TRUE;
  }
  
  if ($flag_update_js_storage) {
    $js_setting = 'mapViewerDisplayJS';
    tripal_map_update_drupal_add_js_params($js_setting, $featuremap_id, $linkage_group, $features_reference);
    
    $js_setting_comparison = 'mapViewerDisplayComparisonJS';
    tripal_map_update_drupal_add_js_params($js_setting_comparison, $featuremap_id_comparison, $linkage_group_comparison, $features_comparison);
    
  }

  // always update the show_comparison flag in JS, regardless of whether the submit button is pressed
  // the comparison linkage group should appear immediately without requiring a submit.
  $show_comparison = $form_state['storage']['show_comp'];
  tripal_map_update_drupal_add_js_show_comparison_params($show_comparison);

  // assign the form state color to form_state storage and js- this is redrawn each time even if submit is not pressed
  $js_setting = 'mapViewerDisplayJS';
  $form_state = tripal_map_assign_form_state_marker_color( $js_setting, $marker_types, $form_state, $color_map);
  
  $form_state = tripal_map_assign_form_state_marker_type_state( $js_setting, $marker_types, $form_state);

  return $form;
}


/**
 * function: tripal_map_form_add_organism_bar
 *	 Create the reference toolbar with selectors for organism, map and chromosome
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 * @param $select_options
 * 
 * @return
 *	 The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_add_organism_bar(&$form, &$form_state, $select_options) {

  $map_org = $select_options['map_org'];
  $options_org = $select_options['options_org'];
  $options_map = $select_options['options_map'];
  $options_chr = $select_options['options_chr'];

  $fs_storage =& $form_state['storage'];
  $org_key =& $fs_storage['org_key'];
  $maps_key =& $fs_storage['maps_key'];
  $chr_key =& $fs_storage['chr_key'];

  $form['organism'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_organism" class="TripalMap"><label class="reference">Select Reference Map:</label><a name="map_anchor">',
  	'#suffix' => '</a></div>',
    '#collapsible' => FALSE,
  );

  $form['organism']['so_frame'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_so_frame" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
  
  $form['organism']['so_frame']['select_organism'] = array(
    '#type' => 'select',
    '#title' => t('Species'),
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
    '#options' => $options_org,
    '#multiple' => FALSE,
    '#default_value' => $org_key,
  );

  $form['organism']['so_frame']['map_chr'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="map_chr' . '" class="form-item">',
    '#suffix' => '</div>',
    '#validated' => TRUE,
  );

  $form['organism']['so_frame']['map_chr']['select_map'] = array(
    '#type' => 'select',
    '#title' => t('Map'),
    '#prefix' => '<div id="select_map' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => $fs_storage['maps'],
    '#multiple' => FALSE,
    '#default_value' => $maps_key,
    '#needs_validation' => FALSE,
  );

  $form['organism']['so_frame']['map_chr']['select_chromosome'] = array(
    '#type' => 'select',
    '#title' => t('Linkage Group'),
    '#prefix' => '<div id="select_chromosome' . '" class="form-item">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form',
      'callback' => 'tripal_map_show_map_chr_form_ajax_callback',
      'progress' => array('type' => 'throbber'),
      'method' => 'replace',
    ),
    '#options' => $fs_storage['chr'],
    '#multiple' => FALSE,
    '#default_value' => $chr_key,
  );

  $form['organism']['so_frame']['button_chromosome_mv-submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
    '#name' => ('Submit_Reference'),
    '#attributes' => array(
      'id' => 'chromosome_mv-submit'
    ),
    '#prefix' => '<div class="chromosome_mv-submit-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_submit_rebuild_form_state'),
    '#after_build' => array('tripal_map_draw_mapViewer'),
  );

  $form['#prefix'] = '<div id="show-map-chr-form">';
  $form['#suffix'] = '</div>';

  return $form;

}


/**
 * function: tripal_map_form_add_toolbars
 *	 Add the color, marker state, and show/hide toolbars to the form
 *
 * @param $form
 *   The form to be initialized
 *   
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 *  @param $marker_types
 *  
 *  @param $color_map
 *  
 * @return 
 *   A form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_form_add_toolbars( &$form, &$form_state, &$marker_types, &$color_map) {

  // Add the color bar
  tripal_map_form_add_color_bar($form, $form_state, $marker_types, $color_map);

  // Add the marker display state bar
  $marker_display_state = array('Show' => 'Show', 'Hide' => 'Hide');
  $default_state = reset($form_state['storage']['marker_type_states']); //return the value of the first array element
  tripal_map_form_add_marker_display_state_bar($form, $form_state, $marker_types, $marker_display_state, $default_state);

  // Add the show/hide bar
  tripal_map_form_add_show_hide_bar($form, $form_state);
  return $form;
}


function tripal_map_draw_mapViewer($form, &$form_state) {

  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_map_utils.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/config_mapViewer.js');
  drupal_add_js(drupal_get_path('module', 'tripal_map') . '/theme/js/tripal_mapViewer.js');
  drupal_add_js(array( 'tripal_map' => array('modulePath' => drupal_get_path( 'module', 'tripal_map' )) ),'setting');
  
  return $form;
}


/**
 * function: tripal_map_submit_rebuild_form_state
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
function tripal_map_submit_rebuild_form_state($form, &$form_state) {

  $form_state['rebuild'] = TRUE;
  $js_setting = 'mapViewerDisplayJS';

  // Set the update flag, so that marker types will be refreshed when tripal_map_form_init is triggered next.
  tripal_map_update_storage($form_state, 'flag_update_js_color', TRUE, TRUE);
  tripal_map_update_storage($form_state, 'flag_update_js_state', TRUE, TRUE);
  
   // pass the marker type display states to js
  $my_settings = array('marker_type_display_states' => $form_state['storage']['marker_type_states']);
  drupal_add_js(array($js_setting => $my_settings), 'setting');
    
}

