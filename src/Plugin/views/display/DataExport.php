<?php

namespace Drupal\views_data_export\Plugin\views\display;

use Drupal\Core\Form\FormStateInterface;
use Drupal\rest\Plugin\views\display\RestExport;
use Drupal\views\Entity\View;
use Drupal\views\ViewEntityInterface;
use Drupal\views\ViewExecutable;

/**
 * Provides a data export display plugin.
 *
 * This overrides the REST Export display to make labeling clearer on the admin
 * UI, and to allow attaching of these to other displays.
 *
 * @ingroup views_display_plugins
 *
 * @ViewsDisplay(
 *   id = "data_export",
 *   title = @Translation("Data export"),
 *   help = @Translation("Export the view results to a file. Can handle very large result sets."),
 *   uses_route = TRUE,
 *   admin = @Translation("Data export"),
 *   returns_response = TRUE
 * )
 */
class DataExport extends RestExport {

  /**
   * {@inheritdoc}
   */
  public static function buildResponse($view_id, $display_id, array $args = []) {
    $response = parent::buildResponse($view_id, $display_id, $args);

    // Add the content disposition header if a custom filename has been used.
    // @todo Is there a way to retrieve an already-executed view?
    $view = View::load($view_id);
    /** @var \Drupal\views\Plugin\views\display\DisplayPluginInterface $display */
    $display = $view->getDisplay($display_id);
    if ($display['display_options']['filename']) {
      $response->headers->set('Content-Disposition', 'attachment; filename="' . static::generateFilename($view, $display['display_options']['filename']) . '"');
    }
    return $response;
  }

  /**
   * Given a filename and a view, generate a filename.
   *
   * @param \Drupal\views\ViewExecutable $view
   *   The views executable.
   * @param $filename_pattern
   *   The filename, which may contain replacement tokens.
   * @return string
   *   The filename with any tokens replaced.
   */
  protected static function generateFilename(ViewEntityInterface $view, $filename_pattern) {
    // @todo Support replacement patterns.
    return $filename_pattern;
  }

  /**
   * {@inheritdoc}
   */
  protected function defineOptions() {
    $options = parent::defineOptions();

    $options['displays'] = ['default' => []];

    // Set the default style plugin, and default to fields.
    $options['style']['contains']['type']['default'] = 'data_export';
    $options['row']['contains']['type']['default'] = 'data_field';

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function optionsSummary(&$categories, &$options) {
    parent::optionsSummary($categories, $options);

    $displays = array_filter($this->getOption('displays'));
    if (count($displays) > 1) {
      $attach_to = $this->t('Multiple displays');
    }
    elseif (count($displays) == 1) {
      $display = array_shift($displays);
      $displays = $this->view->storage->get('display');
      if (!empty($displays[$display])) {
        $attach_to = $displays[$display]['display_title'];
      }
    }

    if (!isset($attach_to)) {
      $attach_to = $this->t('None');
    }

    $options['displays'] = array(
      'category' => 'path',
      'title' => $this->t('Attach to'),
      'value' => $attach_to,
    );

    // Add filename to the summary if set.
    if ($this->getOption('filename')) {
      $options['path']['value'] .= $this->t(' (@filename)', ['@filename' => $this->getOption('filename')]);
    }
  }
    /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    parent::buildOptionsForm($form, $form_state);

    // Remove the 'serializer' option to avoid confusion.
    switch ($form_state->get('section')) {
      case 'style':
        unset($form['style']['type']['#options']['serializer']);
        break;

      case 'path':
        $form['filename'] = [
          '#type' => 'textfield',
          '#title' => $this->t('Filename'),
          '#default_value' => $this->options['filename'],
          '#description' => $this->t('The filename that will be suggested to the browser for downloading purposes. You may include replacement patterns from the list below.'),
        ];
        break;

      case 'displays':
        $form['#title'] .= $this->t('Attach to');
        $displays = [];
        foreach ($this->view->storage->get('display') as $display_id => $display) {
          if ($this->view->displayHandlers->has($display_id) && $this->view->displayHandlers->get($display_id)->acceptAttachments()) {
            $displays[$display_id] = $display['display_title'];
          }
        }
        $form['displays'] = [
          '#title' => $this->t('Displays'),
          '#type' => 'checkboxes',
          '#description' => $this->t('The data export icon will be available only to the selected displays.'),
          '#options' => array_map('\Drupal\Component\Utility\Html::escape', $displays),
          '#default_value' => $this->getOption('displays'),
        ];
        break;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function attachTo(ViewExecutable $clone, $display_id, array &$build) {
    $displays = $this->getOption('displays');
    if (empty($displays[$display_id])) {
      return;
    }

    // Defer to the feed style; it may put in meta information, and/or
    // attach a feed icon.
    $clone->setArguments($this->view->args);
    $clone->setDisplay($this->display['id']);
    $clone->buildTitle();
    if ($plugin = $clone->display_handler->getPlugin('style')) {
      $plugin->attachTo($build, $display_id, $clone->getUrl(), $clone->getTitle());
      foreach ($clone->feedIcons as $feed_icon) {
        $this->view->feedIcons[] = $feed_icon;
      }
    }

    // Clean up.
    $clone->destroy();
    unset($clone);
  }

  /**
   * {@inheritdoc}
   */
  public function submitOptionsForm(&$form, FormStateInterface $form_state) {
    parent::submitOptionsForm($form, $form_state);
    $section = $form_state->get('section');
    switch ($section) {
      case 'displays':
        $this->setOption($section, $form_state->getValue($section));
        break;

      case 'path':
        $this->setOption('filename', $form_state->getValue('filename'));
        break;
    }
  }

}
