<?php
/**
 * @file
 *   Creates field class for the tripal map marker map positions field
 *
 * @ingroup tripal_map
 */


class tripal_map__mapviewer_mappos extends ChadoField {

  // Editable static constants - used by the static functions to provide information to Drupal about
  //  the field and it's default widget and formatter.
  public static $default_label = 'MapViewer Marker MapPositions';

  public static $default_description = 'Displays link for genetic marker map position on the MapViewer map';

  public static $default_widget = 'tripal_map__mapviewer_mappos_widget';

  public static $default_formatter = 'tripal_map__mapviewer_mappos_formatter';

  public static $module = 'tripal_map';  // module that manages the field.

  // Global settings accessed within the globalSettingsForm, updated for all fields when the globalSettingsForm is submitted.
  public static $default_settings = [
    'storage' => 'field_chado_storage',
    'searchable_keys' => [],
  ];

  // Instance specific settings accessible within the instanceSettingsForm.
  public static $default_instance_settings = [
    'term_vocabulary' => 'tripal_map',
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_name' => 'mapviewer_mappos',
    // The unique ID or accession of the term.
    'term_accession' => 'mapviewer_mappos',
    'term_fixed' => FALSE,
    // allow site admin to change the term type
    'auto_attach' => FALSE,
    // disable for fields that load slowly. Load asynchronously, not attaching to display or web services
    'chado_table' => 'feature',
    // Instance maps to this Chado table.
    'chado_column' => 'feature_id',
    // Field value comes from this Chado table column
    'base_table' => 'feature',
  ];

  public static $no_ui = FALSE; // do not allow users to create this field or instances of this field through the UI. Instead use field_create_field() or field_create_instance().

  public static $no_data = FALSE;

  /**
   * function: load
   *    Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'];
   *   where:
   *     $entity: entity object to which this field is attached.
   *     $field_name: is the name of this field
   *     'und': language code ('und' == undefined)
   *     0: cardinality incremented by 1 when more than one item is available
   *     'value': key for the value of the field, which holds the marker
   *   features
   *
   * @ingroup tripal_map_includes
   */
  public function load($entity) {

    // ChadoFields automatically load the chado column specified in the default settings above.
    parent::load($entity);

    $field_name = $this->field['field_name'];
    $field_type = $this->field['type'];
    $field_table = $this->instance['settings']['chado_table'];
    $field_column = $this->instance['settings']['chado_column'];
    $base_table = $this->instance['settings']['base_table'];

    $rel_table = $base_table . '_relationship';
    if (!chado_table_exists($rel_table)) {
      return;
    }
    $schema = chado_get_schema($rel_table);
    if (array_key_exists('rank', $schema['fields'])) {
      $options['order_by'] = ['rank' => 'ASC'];
    }
    // Expand the object to include the relationships.
    // Not all tables have the columns named 'subject_id' and 'object_id'.
    // some have variations on that name and we need to determine what they are.
    $fkeys = $schema['foreign keys'];
    $subject_id_key = 'subject_id';
    $object_id_key = 'object_id';
    foreach ($fkeys as $fktable => $details) {
      foreach ($details['columns'] as $fkey_lcolumn => $fkey_rcolumn) {
        if (preg_match('/^subject_.*id/', $fkey_lcolumn)) {
          $subject_id_key = $fkey_lcolumn;
        }
        if (preg_match('/^object_.*id/', $fkey_lcolumn)) {
          $object_id_key = $fkey_lcolumn;
        }
      }
    }
    $options = [
      'return_array' => 1,
      // do not fully recurse just obtain relationship type, object and subject
      'include_fk' => [
        'type_id' => 1,
        $object_id_key => ['type_id' => 1,],
        $subject_id_key => ['type_id' => 1,],
      ],
    ];

    $chado_record = $entity->chado_record;
    $feature_id = $chado_record->feature_id;
    $feature_type = $chado_record->type_id->name;
    $feature_name = $chado_record->name;

    $chado_record = chado_expand_var($chado_record, 'table', $rel_table, $options);
    if (!$chado_record->$rel_table) {
      return;
    }

    // Load the object relationships, where subject is "instance of (type id)" object.
    $table = [];
    if (isset($chado_record->$rel_table->$object_id_key)) {
      $orelationships = $chado_record->$rel_table->$object_id_key;
      foreach ($orelationships as $relationship) {

        $subject = $relationship->$subject_id_key;
        $object = $relationship->$object_id_key;
        $feature_type = $relationship->type_id->name;

        $feature_id = $subject->feature_id;
        $subject_name = $subject->name;
        $object_name = $object->name;

        // QTL: check relationship type_id, if name is 'located_in',
        // check the object for type_id, if name is QTL and use the object feature id to query the mview
        if ($feature_type == 'located_in') {
          if ($object->type_id->name == 'QTL') {
            $feature_id = $object->feature_id;
            $feature_name = $object_name;
          }
        }
        else {
          // else Genetic marker: orelationship may have multiple entries, one per map the marker locus is present in.
          // Check relationship type_id for corresponding name 'instance_of'.
          // If the subject for type_id name is "marker_locus" use that feature_id to
          // query the mview.
          if ($feature_type == 'instance_of') {
            if ($subject->type_id->name == 'marker_locus') {
              $feature_id = $subject->feature_id;
              $feature_name = $subject_name;
            }
          }
        }
        $results = chado_query("
          SELECT map_id, map_name, linkage_group, marker_pos
          FROM {tripal_map_genetic_markers_mview} MV1
          WHERE MV1.marker_locus_id = :f1_id
          UNION
          SELECT map_id, map_name, linkage_group, marker_pos
          FROM {tripal_map_qtl_and_mtl_mview} MV2
          WHERE MV2.feature_id = :f2_id", [
          ':f1_id' => $feature_id,
          'f2_id' => $feature_id,
        ]);

        if ($results) {
          $obj = $results->fetchObject();
          if (!array_key_exists($feature_id, $table)) {
            $table[$feature_id] = [];
          }
          $ft =& $table[$feature_id];
          $ft['feature_type'] = $feature_type;
          $ft['feature_name'] = $feature_name;
          $ft['map_id'] = $obj->map_id;
          $ft['map_name'] = $obj->map_name;
          $ft['linkage_group'] = $obj->linkage_group;
          $ft['subject_name'] = $subject_name; // locus
          $ft['object_name'] = $object_name; // genetic marker
          $ft['marker_pos'] = $obj->marker_pos;
        }
      }
    }
    else {
      // Heritable phenotypic marker: the entity has no object relationships.
      // check the chado_record type_id, if name is heritable phenotypic marker,
      // then use the chado_record feature_id directly in the mview query
      if ($feature_type == "heritable_phenotypic_marker") {
        $results = chado_query("
          SELECT map_id, map_name, linkage_group, marker_pos
          FROM {tripal_map_genetic_markers_mview} MV1
          WHERE MV1.marker_locus_id = :f1_id 
      	  UNION
          SELECT map_id, map_name, linkage_group, marker_pos
          FROM {tripal_map_qtl_and_mtl_mview} MV2
          WHERE MV2.feature_id = :f2_id", [
          'f1_id' => $feature_id,
          'f2_id' => $feature_id,
        ]);

        if ($results) {
          $obj = $results->fetchObject();
          if (!array_key_exists($feature_id, $table)) {
            $table[$feature_id] = [];
          }
          $ft =& $table[$feature_id];
          $ft['feature_type'] = $feature_type;
          $ft['feature_name'] = $feature_name;
          $ft['map_id'] = $obj->map_id;
          $ft['map_name'] = $obj->map_name;
          $ft['linkage_group'] = $obj->linkage_group;
          $ft['marker_pos'] = $obj->marker_pos;
        }
      }
    }

    $entity->{$field_name}['und'][0] = ['value' => []];
    $e =& $entity->{$field_name}['und'][0]['value'];
    foreach ($table as $feature => $tr) {

      $e[$feature] = [
        'map_id' => $tr['map_id'],
        'map_name' => $tr['map_name'],
        'linkage_group' => $tr['linkage_group'],
        'marker_pos' => $tr['marker_pos'],
        'feature_name' => $tr['feature_name'],
      ];
    }
  }

}
