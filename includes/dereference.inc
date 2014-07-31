<?php

/**
 * @file
 * Asset default image : on entity delete
 * Adapted from patch : https://www.drupal.org/files/entityreference-n1368386-47.patch
 * @see https://www.drupal.org/node/1368386
 */

/**
 * Remove references to an entity from specific fields on specific entities.
 * Or, sets back default value (if any)
 * @param int $entity_id Id of entity to dereference.
 * @param string $entity_type What type of entity to remove references from.
 * @param array $ids Ids of the entities to process.
 * @param array $fields Names of fields that should be processed.
 * @return number of processed entities.
 */
function asset_default_image_dereference($entity_id, $entity_type, $ids, $fields, $threshold = NULL) {
  if (!empty($ids)) {
    $threshold = $threshold ? $threshold : variable_get('queue_bulk_threshold', 20);
    // Split large datasets into smaller subsets for processing.
    if (count($ids) > $threshold) {
      $subsets = array_chunk($ids, $threshold);
      $ids = array_shift($subsets);
      foreach ($subsets as $subset) {
        DrupalQueue::get('entityreference_workers', TRUE)
          ->createItem(array('callback' => 'asset_default_image_dereference', 'arguments' => array($entity_id, $entity_type, $subset, $fields)));
      }
    }
    // Load referencing entities of this type.
    $referencing_entities = entity_load($entity_type, $ids);
    foreach ($referencing_entities as $referencing_entity) {
      
      // Modif 2014/07/30 19:41:03 - will need bundle name for getting default value
      list($referencing_entity_id,
        $referencing_entity_rid,
        $referencing_entity_bundle_name) = entity_extract_ids($entity_type, $referencing_entity);
      
      // Loop through the fields for which references were found.
      foreach ($fields as $field_name) {
        
        // Modif 2014/07/30 19:27:12 - get field instance default value
        $field = field_info_field($field_name);
        $instance = field_info_instance($entity_type, $field_name, $referencing_entity_bundle_name);
        
        // NB: gets the FIRST default value only, see $referencing_entity->{$field_name}[$language][$delta] = $default;
        //$default = array();
        //if (!empty($instance['default_value'])) {
        //  $default = array_shift($instance['default_value']);
        //}
        
        // Since this does method does NOT work for image fields,
        // where the default is set on the field rather than the instance.
        // @see https://api.drupal.org/comment/51928#comment-51928
        if ($field['type'] == 'image' && !empty($instance['settings']['default_image'])) {
          $default = (array) file_load($instance['settings']['default_image']);
        }
        else {
          $default = @array_shift(field_get_default_value($entity_type, $entity, $field, $instance));
        }
        
        // Get all the values in the field.
        foreach ($referencing_entity->{$field_name} as $language => $items) {
          // Loop through them to delete the specific value of the field that references the deleted entity.
          foreach($items as $delta => $value) {
            if ($value['target_id'] == $entity_id) {
              
              // Modif 2014/07/30 19:48:56 - reset to field instance default value (if any)
              if (!empty($default)) {
                $referencing_entity->{$field_name}[$language][$delta] = $default;
              }
              
              // Else, remove reference (keep the original patch behavior)
              else {
                unset($referencing_entity->{$field_name}[$language][$delta]);
              }
            }
          }
        }
      }
      // Store the modified entity. This should take care of all modules wanting to know about
      // the changes and of flushing the relevant caches.
      entity_save($entity_type, $referencing_entity);
    }
    // Return the number of entities from which the references have been removed.
    return count($ids);
  }
  // No entities were passed to dereference from.
  return 0;
}


/**
 * Runs a simple job queue worker.
 */
function asset_default_image_queue_worker($data) {
  return call_user_func_array($data['callback'], $data['arguments']);
}


/**
 * Implements hook_cron_queue_info().
 */
function asset_default_image_cron_queue_info() {
  $queues['entityreference_workers'] = array(
    'worker callback' => 'entityreference_queue_worker',
    'time' => 30,
    'reliable' => TRUE,
  );
  return $queues;
}

