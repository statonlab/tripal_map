<?php

/**
 * @file
 * This file contains the functions used for administration of the module
 *
 * @ingroup tripal_map
 */

/**
 * Administrative settings form
 * 
 * function: tripal_map_admin_form
 *	 Initialize the TripalMap Admin form
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @return
 *   The form array for the TripalMap Admin form.
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_admin_form($form, $form_state) {

  //text tutorial link
  $form['general'] = array(
    '#type' => 'fieldset',
    '#title' => 'General',
  );

  $form['general']['tutorial'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tutorial links',
  );

  $form['general']['tutorial']['video_tut_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Video Tutorial Link'),
    '#default_value' => variable_get('tripal_map_video_tutorial_link', t('')),
  );

  $form['general']['tutorial']['text_tut_link'] = array(
    '#type' => 'textfield',
    '#title' => t('Text Tutorial Link'),
    '#default_value' => variable_get('tripal_map_text_tutorial_link', t('')),
  );

  $form['general']['tutorial']['sample_map_tut_link'] = array(
    '#type' => 'textfield',
      '#title' => t('Sample Map Link  (ex. /200 or https://"@path")', array( '@path' => '<full path to map page>')),
    '#default_value' => variable_get('tripal_map_sample_map_tutorial_link', t('')),
  );

  $form['general']['tutorial']['sample_dotplot_tut_link'] = array(
      '#type' => 'textfield',
      '#title' => t('Sample Dot Plot Link  (ex. /200/4963605/201/4944173 <br/> as @example)', 
                  array( '@example' => '/<mapid1>/<lgid1>/<mapid2>/<lgid2>')),
      '#default_value' => variable_get('tripal_map_sample_dotplot_tutorial_link', t('')),
  );
  
  $form['general']['tutorial']['example_exported_figures_tut_link'] = array(
      '#type' => 'textfield',
      '#title' => t('View Example Exported Figures'),
      '#default_value' => variable_get('tripal_map_example_exported_figures_tutorial_link', t('')),
  );
  
  
  
  $form['toolbar'] = array(
    '#type' => 'fieldset',
    '#title' => 'Toolbar',
  );

  $form['toolbar']['lg_display'] = array(
    '#type' => 'fieldset',
    '#title' => 'Linkage group display',
  );

  $form['toolbar']['lg_display']['marker_name_abbrev'] = array(
    '#type' => 'checkbox',
    '#title' => 'Feature name abbreviation',
    '#description' => 'Select this option if you wish to use derived abbreviation of the QTL or heritable phenotypic marker names if none are available in Chado.',
    '#default_value' => variable_get('tripal_map_marker_name_abbrev', TRUE)
  );

  $form['toolbar']['organism'] = array(
    '#type' => 'fieldset',
    '#title' => 'Organism selector display preferences',
  );

  $options_array = array();
  if (variable_get('tripal_map_toolbar_organism_common_name')) {
    array_push($options_array, 'common_name');
  }
  if (variable_get('tripal_map_toolbar_organism_genus_species') ||
    !(variable_get('tripal_map_toolbar_organism_common_name'))) {
    array_push($options_array, 'genus_species');
  }

  $form['toolbar']['organism']['org_selector'] = array(
    '#type' => 'checkboxes',
    '#options' => array(
    'common_name' => st('Common Name'),
    'genus_species' => st('Genus and Species'),
    ),
    '#default_value' => $options_array,
  );

  $form['chado'] = array(
    '#type' => 'fieldset',
    '#title' => 'Chado',
  );

  $form['chado']['marker_pos_type'] = array(
    '#type' => 'fieldset',
    '#title' => 'Feature Position Type',
  );

  $form['chado']['marker_pos_type']['marker_start_pos_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Feature Start Position Name'),
    '#default_value' => variable_get('tripal_map_chado_marker_start_pos_name', t('')),
  );

  $form['chado']['marker_pos_type']['marker_stop_pos_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Feature Stop Position Name'),
    '#default_value' => variable_get('tripal_map_chado_marker_stop_pos_name', t('')),
  );

  $form['chado']['marker_pos_type']['marker_qtl_peak_pos_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Feature QTL Peak Position Name'),
    '#default_value' => variable_get('tripal_map_chado_marker_qtl_peak_pos_name', t('')),
  );

  $form['additional'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tools',
  );
  
  $form['additional']['map_overview'] = array(
    '#type' => 'fieldset',
    '#title' => 'Map Overview Display',
  );
  
  $form['additional']['map_overview']['map_overview_exclude_maps'] = array(
    '#type' => 'textarea',
    '#title' => t('Exclude these maps from display  (ex. map1; map2; map3)'),
    '#default_value' => variable_get('tripal_map_map_overview_exclude_maps', t('')),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Configuration'
  );

  return $form;
}


/**
 * function: tripal_map_admin_form_validate
 *	 Validate inputs for the TripalMap admin form
 *
 * @param $form
 *   The form to be initialized
 *
 * @param $form_state
 *   The current state of the form. The values and storage that were submitted
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_admin_form_validate($form, &$form_state) {

  // Ensure at least one of the organism selector display preferences checkboxes is selected
  if ($error = !($form_state['values']['org_selector']['common_name'] || 
      $form_state['values']['org_selector']['genus_species'])) {
    form_error($form['toolbar']['organism']['org_selector'], st('Please select one of the Organism Selector Display Preferences checkboxes'));
  }

}


/**
 * function: tripal_map_admin_form_submit
 *	 Called on Admin form submit, write the form state values to Drupal variables 
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
function tripal_map_admin_form_submit($form, &$form_state) {

  variable_set('tripal_map_video_tutorial_link', $form_state['values']['video_tut_link']);
  variable_set('tripal_map_text_tutorial_link', $form_state['values']['text_tut_link']);
  variable_set('tripal_map_sample_map_tutorial_link', $form_state['values']['sample_map_tut_link']);
  variable_set('tripal_map_sample_dotplot_tutorial_link', $form_state['values']['sample_dotplot_tut_link']);
  variable_set('tripal_map_example_exported_figures_tutorial_link', $form_state['values']['example_exported_figures_tut_link']);
  variable_set('tripal_map_marker_name_abbrev', $form_state['values']['marker_name_abbrev']);
  variable_set('tripal_map_toolbar_organism_common_name', $form_state['values']['org_selector']['common_name']);
  variable_set('tripal_map_toolbar_organism_genus_species', $form_state['values']['org_selector']['genus_species']);
  variable_set('tripal_map_chado_marker_start_pos_name', $form_state['values']['marker_start_pos_name']);
  variable_set('tripal_map_chado_marker_stop_pos_name', $form_state['values']['marker_stop_pos_name']);
  variable_set('tripal_map_chado_marker_qtl_peak_pos_name', $form_state['values']['marker_qtl_peak_pos_name']);
  variable_set('tripal_map_map_overview_exclude_maps', $form_state['values']['map_overview_exclude_maps']);

}

