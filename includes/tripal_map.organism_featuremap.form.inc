<?php

/**
 * Provides a form for the map overview field when attached to organism.
 * It displays a select box of each featuremap for that organism, or,
 * just the single featuremap if there is only one.
 *
 * @param $form
 * @param $form_state
 * @param $select - formatted select options of the style [featuremap_id => featuremap name ]
 */
function tripal_map_organism_featuremap_selector($form, &$form_state, $select) {

  $selected = 0;

  if (isset($form_state['values']['featuremap_select'])) {
    $selected = isset($form_state['values']['featuremap_select']);
  }

  $form['rendered_maps'] = [
    '#type' => 'fieldset',
    '#collapsible' => FALSE,
    '#prefix' => '<div id="tripal_map-featuremap-organism-selector-wrapper">',
    '#suffix' => '</div>',
  ];

  $form['rendered_maps']['featuremap_select'] = [
    '#type' => 'select',
    '#options' => $select,
    '#title' => 'Please select a map to view',
    '#default_value' => $selected,
    '#ajax' => [
      'callback' => 'tripal_map_organism_featuremap_callback',
      'wrapper' => 'tripal_map-featuremap-organism-selector-wrapper',
      'effect' => 'fade',
    ],
  ];

  $chosen = 0;
  if (isset($form_state['values']['featuremap_select'])) {
    $chosen = $form_state['input']['featuremap_select'];
  }

  if ($chosen != 0) {
    $mini_form = tripal_map_genetic_map_overview_form([], $form_state, $chosen);
    $form['rendered_maps']['map'] = $mini_form;
    return $form;
  }

  return $form;
}

/**
 * AJAX callback for the organism select box.
 *
 * @param $form
 * @param $form_state
 *
 * @return mixed
 */
function tripal_map_organism_featuremap_callback($form, &$form_state) {

  return $form['rendered_maps'];
}
