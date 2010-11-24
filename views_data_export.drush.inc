<?php

/**
 * Implementation of hook_drush_command().
 */
function views_data_export_drush_command() {
  $items = array();

  $items['views-data-export'] = array (
    'aliases' => array (
      'vde',
    ),
    'description' => 'Fully executes a views_data_export display of a view and writes the output to file.',
    'arguments' => array (
      'view_name' => 'The name of the view',
      'display_id' => 'The id of the views_data_export display to execute on the view',
      'output_file' => 'The file to write the results to - will be overwritten if it already exists',
    ),
    'examples' => array (
      'drush views-data-export myviewname views_data_export_1 output.csv' => 'Export myviewname:views_data_export_1 and write the output to output.csv in the current directory',
    ),
    'drupal dependencies' => array (
      'views_data_export',
    ),
    'core' => array('6'),
  );

  return $items;
}


/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function views_data_export_drush_help($section) {
  switch ($section) {
    case 'drush:views-data-export':
      return dt("This command may be used to fully execute a views_data_export display of a view, batched if need be, and write the output to a file.");
  }
}

/**
 * Implementation of drush_hook_COMMAND_validate().
 */
function drush_views_data_export_validate() {

  $args = drush_get_arguments();
  array_shift($args);

  if (count($args) !== 3) {
    return drush_set_error('ARGUMENTS_REQUIRED', dt('All arguments are required.'));
  }

  if (!$view = views_get_view($args[0])) {
    return drush_set_error('VIEW_DOES_NOT_EXIST', dt('The view !view does not exist.', array ('!view' => $args[0])));
  }

  if (!$view->set_display($args[1])) {
    return drush_set_error('VIEW_DOES_NOT_EXIST', dt('The view !view does not have the !display display.', array ('!view' => $args[0], '!display' => $args[1])));
  }
}

/**
 * Drush command callback to export a views data to a file.
 *
 * @see drush_views_data_export_validate().
 * @see views_data_export_views_data_export_batch_alter().
 */
function drush_views_data_export($view_name, $display_id, $output_file) {

  $view = views_get_view($view_name);
  $view->set_display($display_id);

  // We execute the view normally, and take advantage
  // of an alter function to interject later and batch it ourselves

  $options = array (
    'output_file' => realpath(drush_get_context('DRUSH_OLDCWD', getcwd())) . '/' . $output_file,
  );
  _drush_views_data_export_override_batch($view_name, $display_id, $options);
  $view->execute_display($display_id);
}


/**
 * Helper function that indicates that we want to
 * override the batch that the views_data_export view creates
 * on it's initial time through.
 *
 * Also provides a place to stash options that need to stay around
 * until the end of the batch
 */
function _drush_views_data_export_override_batch($view = NULL, $display = NULL, $options = TRUE) {
  static $_views;
  if (isset($view)) {
    $_views[$view][$display] = $options;
  }
  return $_views;
}

/**
 * Implementation of hook_views_data_export_batch_alter()
 */
function views_data_export_views_data_export_batch_alter(&$batch, &$final_destination, &$querystring) {

  // Copy the batch, because we're going to monkey with it, a lot!
  $new_batch = $batch;

  $view_name = $new_batch['view_name'];
  $display_id = $new_batch['display_id'];

  $ok_to_override = _drush_views_data_export_override_batch();

  // Make sure we do nothing if we are called not following the execution of
  // our drush command. This could happen if the file with this function in it
  // is included during the normal execution of the view
  if (!$ok_to_override[$view_name][$display_id]) {
    return;
  }

  $options = $ok_to_override[$view_name][$display_id];

  // We actually never return from the drupal_alter, but
  // use drush's batch system to run the same batch

  // Add a final callback
  $new_batch['operations'][] = array(
    '_drush_views_data_export_batch_finished', array($batch['eid'], $options['output_file']),
  );

  batch_set($new_batch);
  $new_batch =& batch_get();
  // Drush handles the different processes, so instruct BatchAPI not to.
  $new_batch['progressive'] = FALSE;
  // Process the batch using drush.
  drush_backend_batch_process();

  // Instruct the view display plugin that it shouldn't set a batch.
  $batch = array();
}


/**
 * Get's called at the end of the drush batch process that generated our export
 */
function _drush_views_data_export_batch_finished($eid, $output_file, &$context) {
  // Fetch export info
  $export = views_data_export_get($eid);

  // Perform cleanup
  $view = views_data_export_view_retrieve($eid);
  $view->set_display($export->view_display_id);
  $view->display_handler->batched_execution_state = $export;
  $view->display_handler->remove_index();

  // Get path to temp file
  $temp_file = $view->display_handler->outputfile_path();

  // Copy file over
  if (@drush_op('copy', $temp_file, $output_file)) {
    drush_log("Data export saved to " . $output_file, 'success');
  }
  else {
    drush_set_error('VIEWS_DATA_EXPORT_COPY_ERROR', dt("The file could not be copied to the selected destination"));
  }

}
