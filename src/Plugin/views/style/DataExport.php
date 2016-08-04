<?php

namespace Drupal\views_data_export\Plugin\views\style;

use Drupal\Component\Utility\Html;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Url;
use Drupal\rest\Plugin\views\style\Serializer;

/**
 * A style plugin for data export views.
 *
 * @ingroup views_style_plugins
 *
 * @ViewsStyle(
 *   id = "data_export",
 *   title = @Translation("Data export"),
 *   help = @Translation("Configurable row output for data exports."),
 *   display_types = {"data"}
 * )
 */
class DataExport extends Serializer {

  /**
   * {@inheritdoc}
   */
  public function defineOptions() {
    $options = parent::defineOptions();

    // CSV options.
    // @todo Can these somehow be moved to a plugin?
    $options['csv_settings']['contains'] = [
      'delimiter' => ['default' => ','],
      'enclosure' => ['default' => '"'],
      'escape_char' => ['default' => '\\'],
      'strip_tags' => ['default' => TRUE],
      'trim' => ['default' => TRUE],
      'encoding' => ['default' => 'utf8'],
    ];

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function optionsSummary(&$categories, &$options) {
    parent::optionsSummary($categories, $options);
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    parent::buildOptionsForm($form, $form_state);

    switch ($form_state->get('section')) {
      case 'style_options':
        // CSV options.
        // @todo Can these be moved to a plugin?
        $csv_options = $this->options['csv_settings'];
        $form['csv_settings'] = [
          '#type' => 'fieldset',
          '#title' => $this->t('CSV Settings'),
          '#tree' => TRUE,
          '#states' => [
            'visible' => [':input[name="style_options[formats][csv]"]' => ['checked' => TRUE]],
          ],
          'delimiter' => [
            '#type' => 'textfield',
            '#title' => $this->t('Delimiter'),
            '#description' => $this->t('Indicates the character used to delimit fields. Defaults to a comma (<code>,</code>). For tab-separation use <code>\t</code> characters.'),
            '#default_value' => $csv_options['delimiter'],
          ],
          'enclosure' => [
            '#type' => 'textfield',
            '#title' => $this->t('Ensclosure'),
            '#description' => $this->t('Indicates the character used for field enclosure. Defaults to a double quote (<code>"</code>).'),
            '#default_value' => $csv_options['enclosure'],
          ],
          'escape_char' => [
            '#type' => 'textfield',
            '#title' => $this->t('Escape Character'),
            '#description' => $this->t('Indicates the character used for escaping. Defaults to a backslash (<code>\</code>).'),
            '#default_value' => $csv_options['escape_char'],
          ],
          'strip_tags' => [
            '#type' => 'checkbox',
            '#title' => $this->t('Strip HTML'),
            '#description' => $this->t('Strips HTML tags from CSV cell values.'),
            '#default_value' => $csv_options['strip_tags'],
          ],
          'trim' => [
            '#type' => 'checkbox',
            '#title' => $this->t('Trim Whitespace'),
            '#description' => $this->t('Trims whitespace from beginning and end of CSV cell values.'),
            '#default_value' => $csv_options['trim'],
          ],
          'encoding' => [
            '#type' => 'radios',
            '#title' => $this->t('Encoding'),
            '#description' => $this->t('Determines the encoding used for CSV cell values.'),
            '#options' => [
              'utf8' => $this->t('UTF-8'),
            ],
            '#default_value' => $csv_options['encoding'],
          ],
        ];
    }
  }

  /**
   * {@inheritdoc}
   *
   * @todo This should implement AttachableStyleInterface once
   * https://www.drupal.org/node/2779205 lands.
   */
  public function attachTo(array &$build, $display_id, Url $url, $title) {
    // @todo This mostly hard-codes CSV handling. Figure out how to abstract.

    $url_options = [];
    $input = $this->view->getExposedInput();
    if ($input) {
      $url_options['query'] = $input;
    }
    $url_options['absolute'] = TRUE;

    $url = $url->setOptions($url_options)->toString();

    // Add the CSV icon to the view.
    $type = $this->displayHandler->getContentType();
    $this->view->feedIcons[] = [
      '#theme' => 'feed_icon',
      '#url' => $url,
      '#title' => $title,
      '#theme_wrappers' => [
        'container' => [
          '#attributes' => [
            'class' => [
              Html::cleanCssIdentifier($type) . '-feed',
              'views-data-export-feed',
            ],
          ],
        ],
      ],
      '#attached' => [
        'library' => [
          'views_data_export/views_data_export',
        ],
      ],
    ];

    // Attach a link to the CSV feed, which is an alternate representation.
    $build['#attached']['html_head_link'][][] = [
      'rel' => 'alternate',
      'type' => $this->displayHandler->getMimeType(),
      'title' => $title,
      'href' => $url,
    ];
  }

}
