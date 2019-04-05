<?php


/**
 * @file
 *	 File to create marker visibility toolbar for form that displays genetic maps
 *	 Draw the MapViewer page displaying a chromosome with zoomed view and toolbars below.
 *	 Toolbars select organism, map and linkage group and configure color and marker visibility.
 *
 * @ingroup tripal_map
 */


/**
 * function: tripal_map_form_add_marker_display_state_bar
 *	 Initialize marker visibility display state bar and add to the MapViewer form
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @param $marker_types
 * @param $marker_display_state
 * @param $default_state
 *
 * @return
 *   The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
 function tripal_map_form_add_marker_display_state_bar(&$form, &$form_state, $marker_types, $marker_display_state, $default_state) {


  $form['display_settings']['ds_frame']['marker_mapViewer_display'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'marker_display_fieldset_mapViewer',
      'class' => array("TripalMap")),
    '#collapsible' => FALSE,
  );

  $key_array = array_keys($marker_types);
  $first_key = $key_array[0]; 
  $form['display_settings']['ds_frame']['marker_mapViewer_display']['select_mapViewer_marker_type'] = array(
    '#type' => 'select',
    '#title' => t('Marker visibility'),
    '#ajax' => array(
      'wrapper' => 'select_marker_display_state_mapViewer',
      'callback' => 'tripal_map_show_marker_type_state',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
    '#options' => $marker_types,
    '#multiple' => TRUE,
    '#size' => 3,
    '#default_value' => $marker_types[$first_key],
  );

  $form['display_settings']['ds_frame']['marker_mapViewer_display']['select_mapViewer_display_state'] = array(
    '#type' => 'select',
    '#attributes' => array(
      'id' => 'select_marker_display_state_mapViewer',
    ),
    '#options' => $marker_display_state,
    '#multiple' => FALSE,
    '#default_value' => $default_state,
  );

  $form['display_settings']['ds_frame']['marker_mapViewer_display']['button_mapViewer_marker-submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#name' => ('Submit1_'),
    '#attributes' => array(
      'id' => 'marker-submit_mapViewer'
    ),
    '#prefix' => '<div class="submit_mapViewer-marker-display-state-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_set_marker_type_state'),
  );

  $form['display_settings']['ds_frame']['marker_mapViewer_display']['button_mapViewer-marker-reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#name' => ('Reset1_'),
    '#attributes' => array(
      'id' => 'marker-reset'
    ),
    '#prefix' => '<div class="reset_mapViewer-marker-display-state-button">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'select_marker_display_state_mapViewer',
      'callback' => 'tripal_map_show_marker_type_state',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
    '#submit' => array('tripal_map_reset_marker_type_state'),
  );
 
  return $form;

 }


/**
 * function: tripal_map_show_marker_type_state
 *	 for all views: overview, comparison
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @return
 *   Subsection of the form MapViewer displaying the marker type state.
 *
 * @ingroup tripal_map_includes
 */
 function tripal_map_show_marker_type_state($form, $form_state) {

  $marker_type = "";
  $form_select_mt = $form['display_settings']['ds_frame']['marker_mapViewer_display']['select_mapViewer_marker_type'];
  if (array_key_exists('#input', $form_select_mt) && ($form_select_mt['#input'] == TRUE)) {
    
    // $form_select value is now an array of marker types, choose the first one.
    $key_array = array_keys($form_select_mt['#value']);
    $marker_type = $key_array[0];
  }
  else {
    $marker_type = $form_select_mt['#default_value'];
  }

  $marker_display_state = $form_state['storage']['marker_type_states'][$marker_type];
  $form['display_settings']['ds_frame']['marker_mapViewer_display']['select_mapViewer_display_state']['#value'] = $marker_display_state;

  return $form['display_settings']['ds_frame']['marker_mapViewer_display']['select_mapViewer_display_state'];

 }


/**
 * function: tripal_map_set_marker_type_state
 *	 Assign new states for marker types and rebuild the form state
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @ingroup tripal_map_includes
 */
 function tripal_map_set_marker_type_state($form, &$form_state) {

  // there can be multiple marker_types selected, but o_mapViewer'
  $marker_types = $form_state['values']['select_mapViewer_marker_type'];
  $display_state = $form_state['values']['select_mapViewer_display_state'];

  // preserve the user's setting for marker type states in form_state storage.
  // iterate through all the selected marker types
  foreach ($marker_types as $marker_type_key => $marker_type_value ) {
    $form_state['storage']['marker_type_states'][$marker_type_key] = $display_state;
  }
  
  $form_state['rebuild'] = TRUE;
  tripal_map_submit_rebuild_form_state($form, $form_state);
  
}


/**
 * function: tripal_map_reset_marker_type_state
 *	 Reset states for marker types and rebuild the form state
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @ingroup tripal_map_includes
 */
 function tripal_map_reset_marker_type_state($form, &$form_state) {

  // reset the marker type states to the initial settings.
  $form_state['storage']['marker_type_states'] = $form_state['storage']['marker_type_states_init'];
  $form_state['rebuild'] = TRUE;

  // refresh the selected marker type color selection with the reset value
  tripal_map_submit_rebuild_form_state($form, $form_state);
}


/**
 * function: tripal_map_assign_form_state_marker_type_state
 *	 Set new marker type state if update js storage flag is TRUE
 * 
 * @param $js_setting
 * @param $marker_types
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @return
 *   Updated form state.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_assign_form_state_marker_type_state( $js_setting, $marker_types, &$form_state) {

  $flag_update_js_storage = FALSE;
  if (array_key_exists('flag_update_js_state', $form_state['storage'])) {
    $flag_update_js_storage = $form_state['storage']['flag_update_js_state'];
  }

  // There are no marker types available
  if (count($marker_types) <= 0) {
    if (!(array_key_exists('storage', $form_state))) {
      $form_state['storage'] = array();
    }

    if (!(array_key_exists('marker_type_states', $form_state['storage']))) {
      $form_state['storage']['marker_type_states'] = array();
    }
    if ($flag_update_js_storage) {
      // moved this to submit_rebuild_form_state
      $my_settings = array('marker_type_display_states' => $form_state['storage']['marker_type_states']);
      drupal_add_js(array($js_setting => $my_settings), 'setting');
      $form_state['storage']['flag_update_js_state'] = FALSE;
      
    }

  return $form_state;
  }

  // assign marker display state for each marker type	if not already initialized
  if (!(array_key_exists('storage', $form_state))) {
    // this is the first time the form is loaded. Initialize the marker type states in the form_state storage
    $form_state['storage'] = array();
  }

  $marker_type_settings = array("RFLP" => "Show", "RAPD" => "Show", "CAPS" => "Show", "STS" => "Show",
    "STMS" => "Show", "SNP" => "Show", "COS" => "Show", "SSR" => "Show", "ISSR" => "Show", "RGA" => "Show" );

  // Start a fresh array for marker type states and overwrite storage later, as we may have more or less states
  // than before
  $marker_types_array = array();
  $marker_types_init_array = array();
  foreach ($marker_types as $marker_type) {
    // hard coded marker type settings - this may come from the admin settings eventually
    if (array_key_exists($marker_type, $marker_type_settings)) {
      $marker_types_array[$marker_type] = $marker_type_settings[$marker_type];
    }
    else {
      $marker_types_array[$marker_type] = "Show";
    }
    $marker_types_init_array[$marker_type] = $marker_types_array[$marker_type];

    // Check if the marker type already has a setting in storage, and preserve this setting. Do not assign
    // to the init array
    if (array_key_exists('marker_type_states', $form_state['storage'])) {
      if (array_key_exists($marker_type, $form_state['storage']['marker_type_states'])) {
        $marker_types_array[$marker_type] = $form_state['storage']['marker_type_states'][$marker_type];
      }
    }
  }

  // The marker types can change as new comparison maps are added or removed, so add/remove any types
  // that do not match marker type states and init
  $form_state['storage']['marker_type_states'] = $marker_types_array;
  $form_state['storage']['marker_type_states_init'] = $marker_types_init_array;

  if ($flag_update_js_storage) {
    // pass the marker type display states to js -> move to submit_rebuild_form_state
    $my_settings = array('marker_type_display_states' => $form_state['storage']['marker_type_states']);
    drupal_add_js(array($js_setting => $my_settings), 'setting');
    $form_state['storage']['flag_update_js_state'] = FALSE;
    
  }
  return $form_state;
}

