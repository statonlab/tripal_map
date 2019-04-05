<?php

/**
 * @file
 *	 File to create color toolbar for form that displays genetic maps
 *	 Draw the MapViewer page displaying a chromosome with zoomed view and toolbars below.
 *	 Toolbars select organism, map and linkage group and configure color and marker visibility.
 *
 * @ingroup tripal_map
 */


/**
 * function: tripal_map_show_marker_type_color
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @return
 *   A subsection of the form array for the MapViewer form, acting on the color picker.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_show_marker_type_color($form, $form_state) {

  $marker_type = $form['display_settings']['ds_frame']['color_mapViewer']['select_color_marker_type_mapViewer']['#value'];
  $marker_display_color = $form_state['storage']['marker_type_color_map'][$marker_type];

  $form['display_settings']['ds_frame']['color_mapViewer']['color_picker_mapViewer']['#value'] = $marker_display_color;
  return $form['display_settings']['ds_frame']['color_mapViewer']['color_picker_mapViewer'];
 
}


/**
 * function: tripal_map_form_add_color_bar
 *   Initialize the color toolbar and add it to the MapViewer form
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 * @param $marker_types
 * @param $color_map
 *
 * @return
 *   The form array for the MapViewer form.
 *
 * @ingroup tripal_map_includes
 */
 function tripal_map_form_add_color_bar(&$form, &$form_state, $marker_types, $color_map) {

  $form['display_settings'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_display_settings" class="TripalMap"><label class="reference">Display Settings:</label>',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );
 	
  $form['display_settings']['ds_frame'] = array(
    '#type' => 'container',
    '#prefix' => '<div id="select_ds_frame" class="TripalMap">',
    '#suffix' => '</div>',
    '#collapsible' => FALSE,
  );

  $form['display_settings']['ds_frame']['color_mapViewer'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'color_fieldset_mapViewer',
      'class' => array("TripalMap"),
    ),
  '#collapsible' => FALSE,
  );

  $form['display_settings']['ds_frame']['color_mapViewer']['select_color_marker_type_mapViewer'] = array(
    '#type' => 'select',
    '#title' => t('Marker color'),
    '#ajax' => array(
      'wrapper' => 'select_marker_type_color_mapViewer',
      'callback' => 'tripal_map_show_marker_type_color',
      'method' => 'replace',
      'progress' => array('type' => 'throbber'),
    ),
    '#options' => $marker_types,
    '#multiple' => FALSE,
  );

  $marker_type_color_options = array();
  foreach ($color_map as $color => $hex_code) {
    $marker_type_color_options[$color] = $color;
  }

  // set the default color to that of the first marker type
  $default_color = reset($form_state['storage']['marker_type_color_map']);
  $form['display_settings']['ds_frame']['color_mapViewer']['color_picker_mapViewer'] = array(
    '#type' => 'select',
    '#attributes' => array('id' => 'select_marker_type_color_mapViewer'),
    '#options' => $marker_type_color_options,
    '#multiple' => FALSE,
    '#default_value' => $default_color,
  );

  $form['display_settings']['ds_frame']['color_mapViewer']['button_color_mapViewer-submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#name' => ('Submit12_'),
    '#attributes' => array('id' => 'color_mapViewer-submit'),
    '#prefix' => '<div class="color_mapViewer-submit-button">',
    '#suffix' => '</div>',
    '#submit' => array('tripal_map_set_marker_type_color'),
  );

  $form['display_settings']['ds_frame']['color_mapViewer']['button_color_mapViewer-reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#name' => ('Reset_'),
    '#attributes' => array('id' => 'color_mapViewer-reset'),
    '#prefix' => '<div class="color_mapViewer-reset-button">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'wrapper' => 'show-map-chr-form', 
      'callback' => 'tripal_map_show_marker_type_color', // not called.
      'progress' => array('type' => 'throbber'),
    ),
    '#submit' => array('tripal_map_reset_marker_type_color'),
  );

  return $form;

}


/**
 * function: tripal_map_set_marker_type_color
 *	 Set the marker type color and rebuild the form state
 * 
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *   
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_set_marker_type_color($form, &$form_state) {

  $marker_type = $form_state['values']['select_color_marker_type_mapViewer'];
  $display_color = $form_state['values']['color_picker_mapViewer'];

  // preserve the user's setting for marker type state in form_state build_info.
  $form_state['storage']['marker_type_color_map'][$marker_type] = $display_color;
  $form_state['rebuild'] = TRUE;
  tripal_map_submit_rebuild_form_state($form, $form_state);
}

/**
 * function: tripal_map_reset_marker_type_color
 *	 Reset the marker type color and rebuild the form state
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_reset_marker_type_color($form, &$form_state) {

  // reset the marker type colors to the initial settings.
  $form_state['storage']['marker_type_color_map'] = $form_state['storage']['marker_type_color_map_init'];
  $form_state['rebuild'] = TRUE;

  // refresh the selected marker type color selection with the reset value. <- tried calling through ajax callback
  // but not executed.
  tripal_map_submit_rebuild_form_state($form, $form_state);
 
}


/**
 * function: tripal_map_assign_form_state_marker_color
 *	 for all views: overview, comparison
 * 
 * @param $js_setting
 * @param $marker_types
 * @param &$form_state
 * @param &$color_map
 *   
 * @return
 *   Form state and color map inputs
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_assign_form_state_marker_color($js_setting, $marker_types, &$form_state, &$color_map) {

  // Assign the form state color to form_state storage and js

  $flag_update_js_storage = FALSE;
  if (array_key_exists('flag_update_js_color', $form_state['storage'])) {
    $flag_update_js_storage = $form_state['storage']['flag_update_js_color'];
  }

  if (count($marker_types) <= 0) {
    if (!(array_key_exists('storage', $form_state))) {
      $form_state['storage'] = array();
    }

    if ($flag_update_js_storage) {
      // pass the marker type color map (hex codes) to js
      $marker_type_color_map = array();
      $my_settings = array('marker_type_color_map' => $marker_type_color_map);
      drupal_add_js(array($js_setting => $my_settings), 'setting');
      $form_state['storage']['flag_update_js_color'] = FALSE;
    }

  return $form_state;
  }

  // color map palettes for marker types
  $color_map_vibrant = array("red" => "#FF0000", "green" => "#00FF00", "blue" => "#0000FF", "magenta" => "#FF00FF", "black" => "#000000");
  $color_map_pastel = array("red" => "#F65955", "redDark" => "#940B1C", "orange" => "#F9A34E", "yellow" => "#FFFF67",
    "green" => "#3DFF05", "blue" => "#3950FF", "blueDark" => "#0000FE", "blueEgg" => "#1C80FE", "magenta" => "#F51CFF", "black" => "#3F4043");
  $color_map_csfl = array("salmonPink" => "#EC7063",
    "paleOrange" => "#E59866", "orange" => "#D68910", "redBrown" => "#6E2C00", "eggBlue" => "#2471A3",
    "violetPurple" => "#A569BD", "tealGreen" => "#45B39D", "deepGreen" => "#145A32",
    "avocadoGreen" => "#7D6608", "medGray" => "#566573", "medGray2" => "#5F6A6A", "black" => "#000000");

  // static settings list for marker type colors - this will come from the admin settings eventually
  $color_map = $color_map_csfl;
  
  // CSFL marker type colors
  $marker_type_colors = array("AFLP" => "medGray", "ssAFLP" => "medGray", "RFLP" => "medGray", "ISSR" => "medGray",
    "RAPD" => "medGray", "DAMD" => "medGray", "SCoT" => "medGray", "SRAP" => "medGray", "CAPS" => "eggBlue",
    "dCAPS" => "eggBlue", "CISR" => "salmonPink", "EST marker" => "avocadoGreen", "EST" => "avocadoGreen",
    "EST-SSR" => "avocadoGreen", "gene marker" => "avocadoGreen", "SSR" => "avocadoGreen", "STMS" => "avocadoGreen",
    "STS" => "avocadoGreen", "Indel" => "avocadoGreen", "RGA" => "avocadoGreen", "SCAR" => "avocadoGreen",
    "SNAP" => "paleOrange", "SNP" => "paleOrange", "ASP" => "deepGreen", "DAF" => "deepGreen",
    "DArT" => "deepGreen", "HRM" => "deepGreen", "SSCP" => "deepGreen", "Isozyme" => "deepGreen",
    "genetic marker" => "deepGreen", "QTL" => "orange", "heritable_phenotypic_marker" => "orange");

  // assign colormap entries for the drop list if not already initialized
  if (!(array_key_exists('storage', $form_state))) {
    $form_state['storage'] = array();
  }

  // Start a fresh array for marker type states and overwrite storage later, as we may have more or less states
  // than before
  $marker_type_color_map = array();
  $marker_type_color_map_init = array(); 
  foreach ($marker_types as $marker_type) {
    if (array_key_exists($marker_type, $marker_type_colors)) {
      // assign default colors to marker types
      $marker_type_color_map[$marker_type] = $marker_type_colors[$marker_type];
    }
    else {
      $marker_type_color_map[$marker_type] = "black";
    }
    $marker_type_color_map_init[$marker_type] = $marker_type_color_map[$marker_type];

    // Check if the marker type already has a setting in storage, and preserve this setting. Do not assign it
    // to the init array
    if (array_key_exists('marker_type_color_map', $form_state['storage'])) {
      if (array_key_exists($marker_type, $form_state['storage']['marker_type_color_map'])) {
        $marker_type_color_map[$marker_type] = $form_state['storage']['marker_type_color_map'][$marker_type];
      }
    }
  }

  $form_state['storage']['marker_type_color_map'] = $marker_type_color_map;
  $form_state['storage']['marker_type_color_map_init'] = $marker_type_color_map_init;

  if ($flag_update_js_storage) {
    // convert the colors to hex codes based on the color map
    $marker_type_color_map = array();
    foreach ($form_state['storage']['marker_type_color_map'] as $marker_type => $color) {
      if (!(array_key_exists($marker_type, $marker_type_color_map))) {
        $marker_type_color_map[$marker_type] = $color_map[$color];
      }
    }

    // pass the marker type color map (hex codes) to js - moved to submit_rebuild_form_state()
    $my_settings = array('marker_type_color_map' => $marker_type_color_map);
    drupal_add_js(array($js_setting => $my_settings), 'setting');
    $form_state['storage']['flag_update_js_color'] = FALSE;
    
  }
  
  return $form_state;
}

