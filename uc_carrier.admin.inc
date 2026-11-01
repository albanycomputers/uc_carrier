<?php
/**
 * @file
 * Admin pages for carrier settings, zones and price lists.
 */

/**
 * Form builder: a carrier's services, handling and parcel settings.
 */
function uc_carrier_settings_form($form, &$form_state, $carrier) {
  $settings = uc_carrier_settings($carrier);
  $form_state['carrier'] = $carrier;

  if ($carrier['description']) {
    $form['description'] = array('#markup' => '<p>' . check_plain($carrier['description']) . '</p>');
  }
  $methods = uc_quote_methods(TRUE);
  if (empty($methods['carrier_' . $carrier['id']]['enabled'])) {
    $form['disabled'] = array(
      '#markup' => '<p>' . t('This carrier is not offered at checkout yet: enable it on the <a href="@url">shipping methods</a> page.', array('@url' => url('admin/store/settings/quotes/methods'))) . '</p>',
    );
  }

  $form['services'] = array(
    '#type' => 'fieldset',
    '#title' => t('Services'),
    '#description' => t('Services offered at checkout. A service is only quoted where the active price list has a price for the destination zone, size and weight. Leave a label empty to use "@example".', array('@example' => $carrier['title'] . ' ' . t('Service name'))),
    '#tree' => TRUE,
  );
  foreach ($carrier['services'] as $id => $service) {
    $form['services'][$id]['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => check_plain($service['title']),
      '#description' => check_plain($service['description']),
      '#default_value' => $settings['services'][$id]['enabled'],
    );
    $form['services'][$id]['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label at checkout'),
      '#title_display' => 'invisible',
      '#attributes' => array('placeholder' => t('Label at checkout')),
      '#default_value' => $settings['services'][$id]['label'],
      '#size' => 40,
      '#states' => array(
        'visible' => array(':input[name="services[' . $id . '][enabled]"]' => array('checked' => TRUE)),
      ),
    );
  }

  $form['handling'] = array(
    '#type' => 'fieldset',
    '#title' => t('Handling'),
    '#description' => t('Added to every quote from this carrier: price x (1 + percentage) + fixed fee.'),
  );
  $form['handling']['handling_fixed'] = array(
    '#type' => 'uc_price',
    '#title' => t('Fixed fee'),
    '#default_value' => $settings['handling_fixed'],
  );
  $form['handling']['handling_percent'] = array(
    '#type' => 'number',
    '#title' => t('Percentage'),
    '#step' => 0.01,
    '#field_suffix' => '%',
    '#default_value' => $settings['handling_percent'],
  );

  $form['parcel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Parcel'),
    '#description' => t("The parcel's weight is the products' weight plus packaging. Its size decides the format (letter, parcel...): items are assumed stacked, as long and wide as the largest and as deep as all of them. Products without dimensions use the default size below; with no default, the format is chosen by weight alone, which may under-charge."),
  );
  $form['parcel']['packaging_weight'] = array(
    '#type' => 'number',
    '#title' => t('Packaging weight'),
    '#min' => 0,
    '#field_suffix' => t('g'),
    '#default_value' => $settings['packaging_weight'],
  );
  foreach (array('length' => t('Default item length'), 'width' => t('Default item width'), 'depth' => t('Default item depth')) as $key => $title) {
    $form['parcel']['default_' . $key] = array(
      '#type' => 'number',
      '#title' => $title,
      '#min' => 0,
      '#field_suffix' => t('mm'),
      '#default_value' => $settings['default_' . $key],
    );
  }

  $form['fallback'] = array(
    '#type' => 'fieldset',
    '#title' => t('Fallback'),
    '#description' => t('Offered when no service can take the order (too heavy, no price for the zone). Leave the rate empty to offer nothing.'),
  );
  $form['fallback']['fallback_rate'] = array(
    '#type' => 'uc_price',
    '#title' => t('Rate'),
    '#default_value' => $settings['fallback_rate'],
    '#empty_zero' => FALSE,
  );
  $form['fallback']['fallback_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label at checkout'),
    '#default_value' => $settings['fallback_label'],
    '#attributes' => array('placeholder' => $carrier['title']),
  );

  $form['auto_import'] = array(
    '#type' => 'checkbox',
    '#title' => t('Import new price lists supplied with module updates automatically'),
    '#default_value' => $settings['auto_import'],
    '#description' => t('Supplied lists are the carrier\'s published (retail) prices. Each is added as a new list that takes over on its own date; your existing lists are not changed. Leave unticked if you pay different (online or business) prices: new lists are then offered on the Prices tab instead.'),
  );
  $form['tracking_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Tracking URL'),
    '#description' => t('Tracking numbers on the order page link here. Use [tracking-number] for the number. Leave empty for the default: %default', array('%default' => $carrier['tracking_url'] ? $carrier['tracking_url'] : t('none'))),
    '#default_value' => $settings['tracking_url'],
    '#maxlength' => 512,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Form validation handler for uc_carrier_settings_form().
 */
function uc_carrier_settings_form_validate($form, &$form_state) {
  $url = trim($form_state['values']['tracking_url']);
  if ($url !== '' && !preg_match('#^https?://#i', $url)) {
    form_set_error('tracking_url', t('The tracking URL must start with http:// or https://.'));
  }
  elseif ($url !== '' && strpos($url, '[tracking-number]') === FALSE) {
    form_set_error('tracking_url', t('Put [tracking-number] in the tracking URL where the number goes.'));
  }
}

/**
 * Form submission handler for uc_carrier_settings_form().
 */
function uc_carrier_settings_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $settings = uc_carrier_settings($carrier);
  $values = $form_state['values'];

  foreach ($carrier['services'] as $id => $service) {
    $settings['services'][$id] = array(
      'enabled' => (bool) $values['services'][$id]['enabled'],
      'label' => trim($values['services'][$id]['label']),
    );
  }
  foreach (array('handling_fixed', 'handling_percent', 'packaging_weight', 'default_length', 'default_width', 'default_depth') as $key) {
    $settings[$key] = (float) $values[$key];
  }
  $settings['fallback_rate'] = trim($values['fallback_rate']);
  $settings['fallback_label'] = trim($values['fallback_label']);
  $settings['tracking_url'] = trim($values['tracking_url']);
  $settings['auto_import'] = (bool) $values['auto_import'];

  uc_carrier_settings_save($carrier['id'], $settings);
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Form builder: which zone each of the store's countries is in.
 */
function uc_carrier_zones_form($form, &$form_state, $carrier) {
  $settings = uc_carrier_settings($carrier);
  $form_state['carrier'] = $carrier;

  $form['help'] = array(
    '#markup' => '<p>' . t("The destination zone for each country installed in the store. The carrier's defaults are taken from its own zone list; change a country here if the carrier moves it or you need it treated differently. Countries the carrier does not list go to %zone. Add countries at <a href=\"@url\">Countries</a>.", array(
      '%zone' => isset($carrier['zones'][$carrier['default_zone']]) ? $carrier['zones'][$carrier['default_zone']] : t('no zone (not quoted)'),
      '@url' => url('admin/store/settings/countries'),
    )) . '</p>',
  );

  // The zones themselves: rename or add, e.g. if the carrier adds a zone.
  $form['zone_names'] = array(
    '#type' => 'fieldset',
    '#title' => t('Zone names'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => _uc_carrier_definitions_note($carrier, 'zones', t('zones')) . t('Use the empty row to add a zone, then put countries in it below and add its prices on the Prices tab.'),
  );
  $form['zone_names']['zones'] = array(
    '#theme' => 'uc_carrier_definitions_table',
    '#header' => array(t('Name'), t('Delete')),
    '#tree' => TRUE,
  );
  foreach ($carrier['zones'] + array('_new' => '') as $id => $title) {
    $form['zone_names']['zones'][$id] = array(
      'title' => array(
        '#type' => 'textfield',
        '#title' => t('Name'),
        '#title_display' => 'invisible',
        '#default_value' => $title,
        '#size' => 30,
        '#attributes' => $id == '_new' ? array('placeholder' => t('New zone')) : array(),
      ),
      'delete' => $id == '_new' ? array('#markup' => '') : array('#type' => 'checkbox', '#title' => t('Delete'), '#title_display' => 'invisible'),
    );
  }
  $form['zone_names']['save_names'] = array(
    '#type' => 'submit',
    '#value' => t('Save zone names'),
    '#submit' => array('uc_carrier_zone_names_submit'),
    '#limit_validation_errors' => array(array('zones')),
  );
  $form['zone_names']['reset_names'] = array(
    '#type' => 'submit',
    '#value' => t('Reset zone names to module defaults'),
    '#submit' => array('uc_carrier_definitions_reset'),
    '#limit_validation_errors' => array(),
  );
  $form_state['definitions_key'] = 'zones';
  $form['#attached']['css'][] = backdrop_get_path('module', 'uc_carrier') . '/css/uc_carrier.admin.css';

  $options = array('' => t('- Not delivered -')) + $carrier['zones'];
  $form['countries'] = array(
    '#theme' => 'uc_carrier_zones_table',
    '#tree' => TRUE,
  );
  $countries = db_query('SELECT country_iso_code_2, country_name FROM {uc_countries} ORDER BY country_name')->fetchAllKeyed();
  foreach ($countries as $iso => $name) {
    $default = isset($carrier['countries'][$iso]) ? $carrier['countries'][$iso] : $carrier['default_zone'];
    $form['countries'][$iso] = array(
      '#type' => 'select',
      '#title' => check_plain(t($name)),
      '#options' => $options,
      '#default_value' => isset($settings['countries'][$iso]) ? $settings['countries'][$iso] : $default,
      '#description' => isset($settings['countries'][$iso]) && $settings['countries'][$iso] !== $default ? t('Changed from the default (%zone).', array('%zone' => $default === '' ? t('not delivered') : $carrier['zones'][$default])) : '',
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save zones'),
  );
  $form['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
    '#submit' => array('uc_carrier_zones_form_reset'),
  );

  return $form;
}

/**
 * Form submission handler for uc_carrier_zones_form().
 *
 * Only countries that differ from the carrier's defaults are stored, so a
 * carrier update that moves a country still takes effect.
 */
function uc_carrier_zones_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $settings = uc_carrier_settings($carrier);
  $settings['countries'] = array();
  foreach ($form_state['values']['countries'] as $iso => $zone) {
    $default = isset($carrier['countries'][$iso]) ? $carrier['countries'][$iso] : $carrier['default_zone'];
    if ($zone !== $default) {
      $settings['countries'][$iso] = $zone;
    }
  }
  uc_carrier_settings_save($carrier['id'], $settings);
  backdrop_set_message(format_plural(count($settings['countries']), 'Zones saved; 1 country differs from the defaults.', 'Zones saved; @count countries differ from the defaults.'));
}

/**
 * Submit handler: resets every country to the carrier's default zone.
 */
function uc_carrier_zones_form_reset($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $settings = uc_carrier_settings($carrier);
  $settings['countries'] = array();
  uc_carrier_settings_save($carrier['id'], $settings);
  backdrop_set_message(t('Zones reset to the defaults.'));
}

/**
 * Returns HTML for the zones form: one row per country.
 */
function theme_uc_carrier_zones_table($variables) {
  $element = $variables['element'];
  $rows = array();
  foreach (element_children($element) as $iso) {
    $select = $element[$iso];
    $title = $select['#title'];
    $note = $select['#description'];
    $select['#title_display'] = 'invisible';
    $select['#description'] = '';
    $rows[] = array($title, $iso, backdrop_render($select), $note);
  }
  return theme('table', array(
    'header' => array(t('Country'), t('Code'), t('Zone'), ''),
    'rows' => $rows,
  ));
}

/**
 * Page callback: a carrier's price lists, and the prices in one of them.
 */
function uc_carrier_prices_page($carrier) {
  $lists = uc_carrier_price_lists($carrier['id']);
  $active = uc_carrier_active_price_list($carrier['id']);
  $base = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices';

  if ($warning = uc_carrier_price_age_warning($carrier)) {
    $build['age'] = array('#markup' => '<div class="messages warning">' . $warning . '</div>');
  }
  $build['help'] = array(
    '#markup' => '<p>' . t('A price list applies from its effective date until a later one takes over, so the next list can be prepared ahead of a price change.') . '</p><p>' . t('Prices supplied with the module are correct on the date shown and are not updated by the module maintainers.') . ' ' . uc_carrier_price_update_advice() . '</p><p>' . uc_carrier_prices_warning() . '</p>',
  );
  if (uc_carrier_pending_price_lists($carrier)) {
    $build['pending'] = backdrop_get_form('uc_carrier_pending_form', $carrier);
  }

  $rows = array();
  foreach ($lists as $effective => $count) {
    if ($effective == $active) {
      $status = t('In force');
    }
    elseif ($effective > REQUEST_TIME) {
      $status = t('Scheduled');
    }
    else {
      $status = t('Superseded');
    }
    $rows[] = array(
      $effective ? format_date($effective, 'custom', 'j M Y') : t('Always'),
      $status,
      $count,
      implode(' | ', array(
        l(t('view'), $base, array('query' => array('list' => $effective))),
        l(t('edit'), $base . '/edit/' . $effective),
        l(t('export CSV'), $base . '/export/' . $effective),
        l(t('export JSON'), $base . '/export/' . $effective, array('query' => array('format' => 'json'))),
        l(t('delete'), $base . '/delete/' . $effective),
      )),
    );
  }
  $build['lists'] = array(
    '#theme' => 'table',
    '#header' => array(t('Effective from'), t('Status'), t('Prices'), t('Operations')),
    '#rows' => $rows,
    '#empty' => t('No price list yet. Create or import one to start quoting.'),
  );

  // One service of the shown list at a time, as read-only grids.
  $shown = isset($_GET['list']) && isset($lists[$_GET['list']]) ? (int) $_GET['list'] : $active;
  if ($shown !== FALSE && $lists) {
    $service_id = isset($_GET['service']) && isset($carrier['services'][$_GET['service']]) ? $_GET['service'] : key($carrier['services']);
    $build['shown'] = array(
      '#markup' => '<h3>' . t('Prices from @date', array('@date' => $shown ? format_date($shown, 'custom', 'j M Y') : t('always'))) . '</h3>',
    );
    $build['service'] = uc_carrier_service_select($carrier, $shown, $service_id, url($base, array('query' => array('list' => $shown))));

    list($blocks, $zones_used) = uc_carrier_service_blocks($carrier['id'], $service_id, $shown);
    $zones = array_intersect_key($carrier['zones'], $zones_used);
    foreach ($blocks as $i => $block) {
      $header = array(t('Up to'));
      foreach ($zones as $title) {
        $header[] = check_plain($title);
      }
      $rows = array();
      foreach ($block['bands'] as $weight => $prices) {
        $row = array(uc_weight_format($weight, 'g'));
        foreach (array_keys($zones) as $zone) {
          $row[] = isset($prices[$zone]) ? uc_currency_format($prices[$zone]) : '-';
        }
        $rows[] = $row;
      }
      $build['block_' . $i] = array(
        '#type' => 'fieldset',
        '#title' => check_plain(uc_carrier_block_title($carrier, $block, $service_id)),
        'table' => array(
          '#theme' => 'table',
          '#header' => $header,
          '#rows' => $rows,
          '#attributes' => array('class' => array('uc-carrier-prices-grid')),
        ),
      );
    }
    if (!$blocks) {
      $build['none'] = array('#markup' => '<p>' . t('This service has no prices in this list, so it is not offered.') . '</p>');
    }
  }

  return $build;
}

/**
 * Returns the Service select used on the price pages.
 *
 * Choosing a service reloads $url with ?service= (uc_carrier.admin.js).
 */
function uc_carrier_service_select($carrier, $effective, $service_id, $url) {
  $counts = db_query('SELECT service, COUNT(*) FROM {uc_carrier_rates} WHERE carrier = :carrier AND effective = :effective GROUP BY service', array(
    ':carrier' => $carrier['id'],
    ':effective' => $effective,
  ))->fetchAllKeyed();
  // Select options are escaped when rendered, so the title goes in raw.
  $options = array();
  foreach ($carrier['services'] as $id => $service) {
    $options[$id] = t('!service (@count prices)', array(
      '!service' => $service['title'],
      '@count' => isset($counts[$id]) ? $counts[$id] : 0,
    ));
  }
  return array(
    '#type' => 'select',
    '#title' => t('Service'),
    '#options' => $options,
    '#value' => $service_id,
    '#name' => 'service',
    '#attributes' => array(
      'class' => array('uc-carrier-service-select'),
      'data-url' => $url,
    ),
    '#attached' => array(
      'js' => array(backdrop_get_path('module', 'uc_carrier') . '/js/uc_carrier.admin.js'),
      'css' => array(backdrop_get_path('module', 'uc_carrier') . '/css/uc_carrier.admin.css'),
    ),
  );
}

/**
 * Loads one service's prices in a list, grouped for display as grids.
 *
 * @return array
 *   array($blocks, $zones_used): blocks keyed "format|compensation", each
 *   with 'format', 'compensation' and 'bands' (weight => zone => price,
 *   lightest first); and the zones that have a price, as keys.
 */
function uc_carrier_service_blocks($carrier_id, $service_id, $effective) {
  $result = db_query('SELECT * FROM {uc_carrier_rates} WHERE carrier = :carrier AND effective = :effective AND service = :service ORDER BY format, compensation, max_weight', array(
    ':carrier' => $carrier_id,
    ':effective' => $effective,
    ':service' => $service_id,
  ));
  $blocks = array();
  $zones_used = array();
  foreach ($result as $rate) {
    $compensation = $rate->compensation === NULL ? '' : (string) (float) $rate->compensation;
    $key = $rate->format . '|' . $compensation;
    $blocks[$key]['format'] = $rate->format;
    $blocks[$key]['compensation'] = $compensation;
    $blocks[$key]['bands'][$rate->max_weight][$rate->zone] = (float) $rate->price;
    $zones_used[$rate->zone] = TRUE;
  }

  // The carrier's format order (smallest first), "any size" last, then
  // compensation, lowest first.
  $carrier = uc_carrier_load($carrier_id);
  $order = array_flip(array_keys($carrier['formats']));
  uasort($blocks, function ($a, $b) use ($order) {
    $fa = isset($order[$a['format']]) ? $order[$a['format']] : 999;
    $fb = isset($order[$b['format']]) ? $order[$b['format']] : 999;
    return $fa == $fb ? (float) $a['compensation'] <=> (float) $b['compensation'] : $fa <=> $fb;
  });

  return array($blocks, $zones_used);
}

/**
 * Returns a price block's title: its size and limits, and compensation.
 *
 * The limits are shown as the carrier prints them (cm, kg), so staff need
 * not look them up.
 */
function uc_carrier_block_title($carrier, $block, $service_id = NULL) {
  if ($block['format'] === '') {
    $title = t('Any size');
  }
  else {
    $formats = uc_carrier_service_formats($carrier, $service_id);
    if (!isset($formats[$block['format']])) {
      return t('Deleted size "@id" (ignored)', array('@id' => $block['format']));
    }
    $format = $formats[$block['format']];
    $title = $format['title'];
    $cm = function ($mm) {
      return rtrim(rtrim(number_format($mm / 10, 1, '.', ''), '0'), '.') . 'cm';
    };
    $dims = array();
    foreach (array('length' => 'L', 'width' => 'W', 'depth' => 'D') as $key => $letter) {
      if ($format[$key]) {
        $dims[] = $letter . ': ' . $cm($format[$key]);
      }
    }
    if ($dims) {
      $title .= ' - ' . t('not exceeding: @dims', array('@dims' => implode(' ', $dims)));
    }
    if ($format['weight']) {
      $weight = $format['weight'] >= 1000 ? rtrim(rtrim(number_format($format['weight'] / 1000, 2, '.', ''), '0'), '.') . 'kg' : $format['weight'] . 'g';
      $title .= ' - ' . t('Weight limit: @weight', array('@weight' => $weight));
    }
  }
  if ($block['compensation'] !== '') {
    $title .= ' - ' . t('compensation up to @amount', array('@amount' => uc_currency_format($block['compensation'])));
  }
  return $title;
}

/**
 * Returns the warning not to change carrier prices to allow for packaging.
 */
function uc_carrier_prices_warning() {
  return t("Enter the carrier's own prices here. To allow for packaging or handling, use the handling fee (Settings tab), packaging, or a product's \"Ship as at least\", not these prices: a new price list replaces them from its date.");
}

/**
 * Sets the breadcrumb on price list pages, back to the carrier's Prices tab.
 */
function uc_carrier_prices_breadcrumb($carrier) {
  $base = 'admin/store/settings/quotes/carrier/' . $carrier['id'];
  backdrop_set_breadcrumb(array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Store'), 'admin/store'),
    l(t('Configuration'), 'admin/store/settings'),
    l(t('Shipping quotes'), 'admin/store/settings/quotes'),
    l($carrier['title'], $base),
    l(t('Prices'), $base . '/prices'),
  ));
}

/**
 * Form builder: starts a new price list, empty or copied from another.
 */
function uc_carrier_new_prices_form($form, &$form_state, $carrier) {
  $form_state['carrier'] = $carrier;
  uc_carrier_prices_breadcrumb($carrier);

  $lists = uc_carrier_price_lists($carrier['id']);
  $options = array('' => t('- Start empty -'));
  foreach ($lists as $effective => $count) {
    $options[$effective] = t('Copy the list from @date (@count prices)', array(
      '@date' => $effective ? format_date($effective, 'custom', 'j M Y') : t('always'),
      '@count' => $count,
    ));
  }

  $form['effective'] = array(
    '#type' => 'date',
    '#title' => t('Effective from'),
    '#description' => t('Quotes use the new list from this date until a later list takes over. Enter the carrier\'s price change date to prepare it in advance.'),
    '#default_value' => array('year' => date('Y'), 'month' => date('n'), 'day' => date('j')),
  );
  $active = uc_carrier_active_price_list($carrier['id']);
  $form['copy'] = array(
    '#type' => 'select',
    '#title' => t('Prices'),
    '#options' => $options,
    '#default_value' => $active === FALSE ? '' : $active,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create and edit'),
  );

  return $form;
}

/**
 * Form validation handler for uc_carrier_new_prices_form().
 */
function uc_carrier_new_prices_form_validate($form, &$form_state) {
  $date = $form_state['values']['effective'];
  $effective = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);
  $lists = uc_carrier_price_lists($form_state['carrier']['id']);
  if (isset($lists[$effective])) {
    form_set_error('effective', t('There is already a price list from that date; edit it instead.'));
  }
  $form_state['effective'] = $effective;
}

/**
 * Form submission handler for uc_carrier_new_prices_form().
 */
function uc_carrier_new_prices_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $effective = $form_state['effective'];
  $copy = $form_state['values']['copy'];

  if ($copy !== '') {
    $result = db_query('SELECT service, zone, format, max_weight, price, compensation FROM {uc_carrier_rates} WHERE carrier = :carrier AND effective = :effective', array(
      ':carrier' => $carrier['id'],
      ':effective' => $copy,
    ));
    $query = db_insert('uc_carrier_rates')->fields(array('carrier', 'service', 'zone', 'format', 'max_weight', 'price', 'compensation', 'effective'));
    $count = 0;
    foreach ($result as $rate) {
      $query->values(array($carrier['id'], $rate->service, $rate->zone, $rate->format, $rate->max_weight, $rate->price, $rate->compensation, $effective));
      $count++;
    }
    $query->execute();
    backdrop_set_message(t('Copied @count prices. Change the ones that differ.', array('@count' => $count)));
  }

  $form_state['redirect'] = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices/edit/' . $effective;
}

/**
 * Form builder: edits one service's prices in a price list.
 *
 * Prices are shown as grids, one per format and compensation level: weight
 * bands down, zones across. An empty price means the service is not offered
 * for that zone and band; an empty weight removes the band.
 */
function uc_carrier_prices_edit_form($form, &$form_state, $carrier, $effective) {
  uc_carrier_prices_breadcrumb($carrier);
  $effective = (int) $effective;
  $base = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices';
  $service_id = isset($_GET['service']) && isset($carrier['services'][$_GET['service']]) ? $_GET['service'] : key($carrier['services']);
  $form_state['carrier'] = $carrier;
  $form_state['effective'] = $effective;
  $form_state['service'] = $service_id;

  backdrop_set_title(t('@carrier prices from @date', array(
    '@carrier' => $carrier['title'],
    '@date' => $effective ? format_date($effective, 'custom', 'j M Y') : t('always'),
  )));

  $form['back'] = array(
    '#markup' => '<p>' . l(t('‹ Back to price lists'), $base) . '</p>',
  );
  $form['service'] = uc_carrier_service_select($carrier, $effective, $service_id, url($base . '/edit/' . $effective));
  $form['help'] = array(
    '#markup' => '<div class="messages warning">' . uc_carrier_prices_warning() . '</div><p>' . t('Weights are the top of each band, in grams. Leave a price empty where the service is not offered; clear a weight to remove that band. Use the empty row to add a band.') . '</p>',
  );

  // Existing prices, grouped into blocks by format and compensation.
  list($blocks, $zones_used) = uc_carrier_service_blocks($carrier['id'], $service_id, $effective);
  if (!empty($form_state['extra_blocks'])) {
    foreach ($form_state['extra_blocks'] as $key => $block) {
      $blocks += array($key => $block + array('bands' => array()));
    }
  }

  // Zones with a price, in the carrier's order; all of them on request or
  // when the service has none yet.
  $show_all = !empty($_GET['zones']) || !$zones_used;
  $zones = array();
  foreach ($carrier['zones'] as $id => $title) {
    if ($show_all || isset($zones_used[$id])) {
      $zones[$id] = $title;
    }
  }
  $form_state['zones'] = $zones;
  if (!$show_all) {
    $form['all_zones'] = array(
      '#markup' => '<p>' . l(t('Show all zones'), current_path(), array('query' => array('service' => $service_id, 'zones' => 'all'))) . '</p>',
    );
  }
  elseif ($zones_used && count($zones_used) < count($carrier['zones'])) {
    $form['all_zones'] = array(
      '#markup' => '<p>' . l(t('Show only zones with prices'), current_path(), array('query' => array('service' => $service_id))) . '</p>',
    );
  }

  $formats = $carrier['formats'];
  $form['blocks'] = array('#tree' => TRUE);
  $i = 0;
  foreach ($blocks as $block) {
    $form['blocks'][$i] = array(
      '#type' => 'fieldset',
      '#title' => check_plain(uc_carrier_block_title($carrier, $block, $service_id)),
    );
    $form['blocks'][$i]['format'] = array('#type' => 'value', '#value' => $block['format']);
    $form['blocks'][$i]['compensation'] = array('#type' => 'value', '#value' => $block['compensation']);
    $form['blocks'][$i]['bands'] = array(
      '#theme' => 'uc_carrier_prices_grid',
      '#zones' => $zones,
    );
    $bands = $block['bands'];
    ksort($bands);
    // One empty row for a new band.
    $bands[''] = array();
    $j = 0;
    foreach ($bands as $weight => $prices) {
      $row = &$form['blocks'][$i]['bands'][$j];
      $row['weight'] = array(
        '#type' => 'textfield',
        '#title' => t('Up to (g)'),
        '#title_display' => 'invisible',
        '#default_value' => $weight,
        '#size' => 7,
        '#field_suffix' => t('g'),
      );
      foreach ($zones as $zone => $zone_title) {
        $row['zones'][$zone] = array(
          '#type' => 'textfield',
          '#title' => check_plain($zone_title),
          '#title_display' => 'invisible',
          '#default_value' => isset($prices[$zone]) ? number_format($prices[$zone], 2, '.', '') : '',
          '#size' => 7,
        );
      }
      unset($row);
      $j++;
    }
    $i++;
  }

  $form['add'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add a size or compensation level'),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($blocks),
  );
  $format_options = array('' => t('Any size'));
  foreach ($formats as $id => $format) {
    $format_options[$id] = $format['title'];
  }
  $form['add']['new_format'] = array(
    '#type' => 'select',
    '#title' => t('Size'),
    '#options' => $format_options,
  );
  $form['add']['new_compensation'] = array(
    '#type' => 'textfield',
    '#title' => t('Compensation (cover)'),
    '#field_prefix' => '£',
    '#description' => t('Optional. The cover included at these prices, shown in the heading after the size limits: enter 75 for "compensation up to £75.00". For a service with several cover levels, add one block per level; e.g. Special Delivery by 1pm has £750, £1,000 and £2,500. The cheapest is quoted.'),
    '#size' => 10,
  );
  $form['add']['add_block'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
    '#submit' => array('uc_carrier_prices_edit_form_add_block'),
    '#limit_validation_errors' => array(array('new_format'), array('new_compensation')),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save prices'),
  );
  $form['actions']['back'] = array(
    '#markup' => l(t('Back to price lists'), $base),
  );

  return $form;
}

/**
 * Submit handler: adds an empty block to the price grid.
 */
function uc_carrier_prices_edit_form_add_block($form, &$form_state) {
  $format = $form_state['values']['new_format'];
  $compensation = trim($form_state['values']['new_compensation']);
  $compensation = is_numeric($compensation) ? (string) (float) $compensation : '';
  $form_state['extra_blocks'][$format . '|' . $compensation] = array(
    'format' => $format,
    'compensation' => $compensation,
  );
  $form_state['rebuild'] = TRUE;
}

/**
 * Form validation handler for uc_carrier_prices_edit_form().
 */
function uc_carrier_prices_edit_form_validate($form, &$form_state) {
  if (empty($form_state['values']['blocks'])) {
    return;
  }
  foreach ($form_state['values']['blocks'] as $i => $block) {
    $seen = array();
    foreach ($block['bands'] as $j => $row) {
      $weight = trim($row['weight']);
      $prices = array_filter(array_map('trim', $row['zones']), 'strlen');
      if ($weight === '') {
        continue;
      }
      if (!ctype_digit($weight) || $weight == 0) {
        form_set_error("blocks][$i][bands][$j][weight", t('Weights must be whole grams.'));
      }
      elseif (isset($seen[$weight])) {
        form_set_error("blocks][$i][bands][$j][weight", t('The @weight g band appears twice.', array('@weight' => $weight)));
      }
      $seen[$weight] = TRUE;
      foreach ($prices as $zone => $price) {
        if (!is_numeric($price) || $price < 0) {
          form_set_error("blocks][$i][bands][$j][zones][$zone", t('Prices must be numbers.'));
        }
      }
    }
  }
}

/**
 * Form submission handler for uc_carrier_prices_edit_form().
 *
 * Replaces the service's prices in this list with the grid's contents.
 */
function uc_carrier_prices_edit_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $rows = array();
  if (!empty($form_state['values']['blocks'])) {
    foreach ($form_state['values']['blocks'] as $block) {
      foreach ($block['bands'] as $row) {
        $weight = trim($row['weight']);
        if ($weight === '') {
          continue;
        }
        foreach ($row['zones'] as $zone => $price) {
          if (trim($price) === '') {
            continue;
          }
          $rows[] = array(
            'carrier' => $carrier['id'],
            'service' => $form_state['service'],
            'zone' => $zone,
            'format' => $block['format'],
            'max_weight' => (int) $weight,
            'price' => trim($price),
            'compensation' => $block['compensation'] === '' ? NULL : $block['compensation'],
            'effective' => $form_state['effective'],
          );
        }
      }
    }
  }

  $transaction = db_transaction();
  // Only the zones shown are replaced; hidden zones have no prices anyway.
  db_delete('uc_carrier_rates')
    ->condition('carrier', $carrier['id'])
    ->condition('effective', $form_state['effective'])
    ->condition('service', $form_state['service'])
    ->condition('zone', array_keys($form_state['zones']), 'IN')
    ->execute();
  if ($rows) {
    $query = db_insert('uc_carrier_rates')->fields(array_keys($rows[0]));
    foreach ($rows as $row) {
      $query->values($row);
    }
    $query->execute();
  }
  unset($form_state['extra_blocks']);

  backdrop_set_message(t('Saved @count prices for @service.', array(
    '@count' => count($rows),
    '@service' => $carrier['services'][$form_state['service']]['title'],
  )));
  $form_state['redirect'] = array(current_path(), array('query' => array('service' => $form_state['service'])));
}

/**
 * Returns HTML for a price grid: weight bands down, zones across.
 */
function theme_uc_carrier_prices_grid($variables) {
  $element = $variables['element'];
  $header = array(t('Up to'));
  foreach ($element['#zones'] as $title) {
    $header[] = check_plain($title);
  }
  $rows = array();
  foreach (element_children($element) as $j) {
    $row = array(backdrop_render($element[$j]['weight']));
    foreach (element_children($element[$j]['zones']) as $zone) {
      $row[] = backdrop_render($element[$j]['zones'][$zone]);
    }
    $rows[] = $row;
  }
  return theme('table', array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array('class' => array('uc-carrier-prices-grid')),
  ));
}

/**
 * Form builder: imports a price list from CSV.
 */
function uc_carrier_import_form($form, &$form_state, $carrier) {
  $form_state['carrier'] = $carrier;
  uc_carrier_prices_breadcrumb($carrier);

  $codes = array();
  foreach (array('services' => t('Services'), 'zones' => t('Zones'), 'formats' => t('Formats')) as $key => $title) {
    $items = array();
    foreach ($carrier[$key] as $id => $item) {
      $items[] = '<code>' . check_plain($id) . '</code> ' . check_plain(is_array($item) ? $item['title'] : $item);
    }
    $codes[] = '<strong>' . $title . ':</strong> ' . implode(', ', $items);
  }

  $form['help'] = array(
    '#markup' => '<p>' . t('A JSON price list (as exported here, or supplied with the module), or a CSV file with a header row and the columns <code>service, zone, format, max_weight, price, compensation</code>. Weight is the top of the band in grams; format may be empty for a service priced by weight alone; compensation is optional. Export an existing list for an example.') . '</p><p>' . implode('<br>', $codes) . '</p>',
  );
  $supplied = uc_carrier_supplied_price_lists($carrier);
  if ($supplied) {
    $options = array('' => t('- Upload a file instead -'));
    foreach ($supplied as $date => $list) {
      $options[$date] = $list['title'];
    }
    $form['supplied'] = array(
      '#type' => 'select',
      '#title' => t('Price list supplied with the module'),
      '#options' => $options,
      '#description' => t('Imports with its own effective date, replacing any list from that date.'),
    );
  }
  $form['file'] = array(
    '#type' => 'file',
    '#title' => t('JSON or CSV file'),
    '#required' => FALSE,
  );
  $form['effective'] = array(
    '#type' => 'date',
    '#title' => t('Effective from'),
    '#description' => t('For a CSV file, or a JSON file without its own date. Quotes use this list from this date until a later list takes over. Importing again for the same date replaces that list.'),
    '#default_value' => array('year' => date('Y'), 'month' => date('n'), 'day' => date('j')),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

/**
 * Form validation handler for uc_carrier_import_form().
 */
function uc_carrier_import_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['supplied'])) {
    return;
  }
  $file = file_save_upload('file', array('file_validate_extensions' => array('json csv txt')));
  if (!$file) {
    form_set_error('file', t('Choose a supplied price list, or a JSON or CSV file to import.'));
    return;
  }
  $form_state['upload'] = $file;
}

/**
 * Form submission handler for uc_carrier_import_form().
 */
function uc_carrier_import_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $values = $form_state['values'];

  $date = $values['effective'];
  $effective = mktime(0, 0, 0, $date['month'], $date['day'], $date['year']);

  if (!empty($values['supplied'])) {
    $supplied = uc_carrier_supplied_price_lists($carrier);
    $effective = $supplied[$values['supplied']]['effective'];
    // Chosen deliberately here, so it replaces a list from the same date.
    $result = uc_carrier_handle_supplied_list($carrier, $values['supplied'], TRUE, TRUE);
  }
  else {
    $path = backdrop_realpath($form_state['upload']->uri);
    if ($data = uc_carrier_read_json($path)) {
      if ($data['effective']) {
        $effective = strtotime($data['effective']);
      }
      $result = uc_carrier_import_json($carrier, $data, $effective);
    }
    else {
      $result = uc_carrier_import_csv($carrier, $path, $effective, TRUE);
    }
    file_delete($form_state['upload']->fid);
  }

  if ($result['errors']) {
    foreach (array_slice($result['errors'], 0, 20) as $error) {
      backdrop_set_message($error, 'error');
    }
    backdrop_set_message(t('Nothing imported: fix the file and try again.'), 'error');
    return;
  }
  backdrop_set_message(t('Imported @count prices, effective from @date.', array('@count' => $result['count'], '@date' => format_date($effective, 'custom', 'j M Y'))));
  $form_state['redirect'] = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices';
}

/**
 * Page callback: downloads a price list as CSV, or JSON with ?format=json.
 *
 * A JSON export can be put in a carrier module's prices folder to supply it
 * with the next release.
 */
function uc_carrier_export_csv($carrier, $effective) {
  $lists = uc_carrier_price_lists($carrier['id']);
  if (!isset($lists[$effective])) {
    return MENU_NOT_FOUND;
  }

  if (isset($_GET['format']) && $_GET['format'] == 'json') {
    $date = $effective ? date('Y-m-d', $effective) : '';
    $title = t('@carrier prices from @date', array('@carrier' => $carrier['title'], '@date' => $effective ? format_date($effective, 'custom', 'j F Y') : t('always')));
    backdrop_add_http_header('Content-Type', 'application/json; charset=utf-8');
    backdrop_add_http_header('Content-Disposition', 'attachment; filename="' . $carrier['id'] . '-' . ($date ? $date : 'always') . '.json"');
    print uc_carrier_price_list_json($carrier, $effective, $title);
    backdrop_exit();
  }

  $filename = $carrier['id'] . '-' . ($effective ? date('Y-m-d', $effective) : 'always') . '.csv';
  backdrop_add_http_header('Content-Type', 'text/csv; charset=utf-8');
  backdrop_add_http_header('Content-Disposition', 'attachment; filename="' . $filename . '"');

  $out = fopen('php://output', 'w');
  fputcsv($out, array('service', 'zone', 'format', 'max_weight', 'price', 'compensation'));
  $result = db_query('SELECT * FROM {uc_carrier_rates} WHERE carrier = :carrier AND effective = :effective ORDER BY service, zone, format, max_weight', array(
    ':carrier' => $carrier['id'],
    ':effective' => $effective,
  ));
  foreach ($result as $rate) {
    fputcsv($out, array(
      $rate->service,
      $rate->zone,
      $rate->format,
      $rate->max_weight,
      rtrim(rtrim(number_format($rate->price, 5, '.', ''), '0'), '.'),
      $rate->compensation === NULL ? '' : rtrim(rtrim(number_format($rate->compensation, 5, '.', ''), '0'), '.'),
    ));
  }
  fclose($out);
  backdrop_exit();
}

/**
 * Form builder: confirms deleting a price list.
 */
function uc_carrier_delete_prices_form($form, &$form_state, $carrier, $effective) {
  $form_state['carrier'] = $carrier;
  uc_carrier_prices_breadcrumb($carrier);
  $form_state['effective'] = (int) $effective;

  return confirm_form(
    $form,
    t('Delete the @carrier price list effective from @date?', array(
      '@carrier' => $carrier['title'],
      '@date' => $effective ? format_date($effective, 'custom', 'j M Y') : t('always'),
    )),
    'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices',
    t('If it is the list in force, quotes fall back to the previous one. This cannot be undone; export it first if unsure.'),
    t('Delete')
  );
}

/**
 * Form submission handler for uc_carrier_delete_prices_form().
 */
function uc_carrier_delete_prices_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $count = db_delete('uc_carrier_rates')
    ->condition('carrier', $carrier['id'])
    ->condition('effective', $form_state['effective'])
    ->execute();
  backdrop_set_message(t('Deleted @count prices.', array('@count' => $count)));
  $form_state['redirect'] = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/prices';
}

/**
 * Returns HTML for a definitions table: one row per child, one cell per
 * grandchild.
 */
function theme_uc_carrier_definitions_table($variables) {
  $element = $variables['element'];
  $rows = array();
  foreach (element_children($element) as $key) {
    $row = array();
    foreach (element_children($element[$key]) as $cell) {
      $row[] = backdrop_render($element[$key][$cell]);
    }
    $rows[] = $row;
  }
  return theme('table', array(
    'header' => $element['#header'],
    'rows' => $rows,
    'attributes' => array('class' => array('uc-carrier-definitions')),
  ));
}

/**
 * Returns a note saying whether a carrier's definitions are the module's.
 */
function _uc_carrier_definitions_note($carrier, $key, $what) {
  if (in_array($key, $carrier['customised'])) {
    return '<p><strong>' . t('This site has its own @what.', array('@what' => $what)) . '</strong> ' . t('"Reset to module defaults" brings back the @what supplied with the module.', array('@what' => $what)) . '</p>';
  }
  return '<p>' . t('These are the @what supplied with the module. Change them here if the carrier changes its products before the module is updated.', array('@what' => $what)) . '</p>';
}

/**
 * Returns a small number field for a definitions table.
 */
function _uc_carrier_number_cell($title, $value, $suffix, $placeholder = '') {
  return array(
    '#type' => 'textfield',
    '#title' => $title,
    '#title_display' => 'invisible',
    '#default_value' => $value === '' || $value === NULL ? '' : rtrim(rtrim(number_format((float) $value, 1, '.', ''), '0'), '.'),
    '#size' => 6,
    '#field_suffix' => $suffix,
    '#attributes' => array('class' => array('uc-carrier-number')) + ($placeholder === '' ? array() : array('placeholder' => $placeholder)),
  );
}

/**
 * Validates the number cells of a definitions table.
 */
function _uc_carrier_validate_numbers($rows, $parents, array $keys) {
  foreach ($rows as $i => $row) {
    foreach ($keys as $key) {
      $value = trim($row[$key]);
      if ($value !== '' && (!is_numeric($value) || $value < 0)) {
        form_set_error(implode('][', array_merge($parents, array($i, $key))), t('Sizes and weights must be positive numbers.'));
      }
    }
  }
}

/**
 * Adds Save and Reset buttons to a definitions form.
 */
function _uc_carrier_definitions_actions(&$form) {
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to module defaults'),
    '#submit' => array('uc_carrier_definitions_reset'),
    '#limit_validation_errors' => array(),
  );
  $form['#attached']['css'][] = backdrop_get_path('module', 'uc_carrier') . '/css/uc_carrier.admin.css';
}

/**
 * Submit handler: drops a site's own sizes, services or zone names.
 */
function uc_carrier_definitions_reset($form, &$form_state) {
  uc_carrier_definitions_save($form_state['carrier']['id'], $form_state['definitions_key'], NULL);
  backdrop_set_message(t('Back to the module defaults.'));
}

/**
 * Warns about prices left pointing at a deleted size, service or zone.
 */
function _uc_carrier_orphan_warning($carrier_id, $column, array $keep) {
  $query = db_select('uc_carrier_rates', 'r')->condition('carrier', $carrier_id);
  if ($column == 'format') {
    $keep[] = '';
  }
  if ($keep) {
    $query->condition($column, $keep, 'NOT IN');
  }
  $count = $query->countQuery()->execute()->fetchField();
  if ($count) {
    backdrop_set_message(format_plural($count, '1 price belongs to something deleted and is ignored. It comes back if you add it again with the same name, or reset to the module defaults.', '@count prices belong to something deleted and are ignored. They come back if you add it again with the same name, or reset to the module defaults.'), 'warning');
  }
}

/**
 * Form builder: the carrier's sizes (formats) and their limits.
 */
function uc_carrier_formats_form($form, &$form_state, $carrier) {
  $form_state['carrier'] = $carrier;
  $form_state['definitions_key'] = 'formats';
  $form['note'] = array('#markup' => _uc_carrier_definitions_note($carrier, 'formats', t('sizes')));
  $form['help'] = array(
    '#markup' => '<p>' . t("Smallest first: a parcel gets the first size it fits. Dimensions in mm, weights in grams; leave a limit empty for none. Renaming a size does not affect its prices. Use the empty row to add a size, then add its prices on the Prices tab.") . '</p>',
  );

  $form['formats'] = array(
    '#theme' => 'uc_carrier_definitions_table',
    '#header' => array(t('Order'), t('Name'), t('Length'), t('Width'), t('Depth'), t('Weight limit'), t('Delete')),
    '#tree' => TRUE,
  );
  $rows = $carrier['formats'] + array('_new' => array('title' => '', 'length' => 0, 'width' => 0, 'depth' => 0, 'weight' => 0));
  $i = 0;
  foreach ($rows as $id => $format) {
    $form['formats'][$id] = array(
      'order' => array('#type' => 'textfield', '#title' => t('Order'), '#title_display' => 'invisible', '#default_value' => ++$i, '#size' => 2, '#attributes' => array('class' => array('uc-carrier-number'))),
      'title' => array(
        '#type' => 'textfield',
        '#title' => t('Name'),
        '#title_display' => 'invisible',
        '#default_value' => $format['title'],
        '#size' => 20,
        '#attributes' => $id == '_new' ? array('placeholder' => t('New size')) : array(),
      ),
      'length' => _uc_carrier_number_cell(t('Length'), $format['length'] ? $format['length'] : '', t('mm')),
      'width' => _uc_carrier_number_cell(t('Width'), $format['width'] ? $format['width'] : '', t('mm')),
      'depth' => _uc_carrier_number_cell(t('Depth'), $format['depth'] ? $format['depth'] : '', t('mm')),
      'weight' => _uc_carrier_number_cell(t('Weight limit'), $format['weight'] ? $format['weight'] : '', t('g')),
      'delete' => $id == '_new' ? array('#markup' => '') : array('#type' => 'checkbox', '#title' => t('Delete'), '#title_display' => 'invisible'),
    );
  }

  _uc_carrier_definitions_actions($form);
  return $form;
}

/**
 * Form validation handler for uc_carrier_formats_form().
 */
function uc_carrier_formats_form_validate($form, &$form_state) {
  _uc_carrier_validate_numbers($form_state['values']['formats'], array('formats'), array('length', 'width', 'depth', 'weight'));
}

/**
 * Form submission handler for uc_carrier_formats_form().
 */
function uc_carrier_formats_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $rows = $form_state['values']['formats'];
  uasort($rows, function ($a, $b) {
    return (float) $a['order'] <=> (float) $b['order'];
  });

  $formats = array();
  foreach ($rows as $id => $row) {
    if (!empty($row['delete']) || trim($row['title']) === '') {
      continue;
    }
    if ($id == '_new') {
      $id = uc_carrier_machine_name($row['title'], $carrier['formats'] + $formats);
    }
    $formats[$id] = array(
      'title' => trim($row['title']),
      'length' => (float) $row['length'],
      'width' => (float) $row['width'],
      'depth' => (float) $row['depth'],
      'weight' => (int) $row['weight'],
    );
  }
  uc_carrier_definitions_save($carrier['id'], 'formats', $formats);
  backdrop_set_message(t('Sizes saved.'));
  _uc_carrier_orphan_warning($carrier['id'], 'format', array_keys($formats));
}

/**
 * Form builder: the carrier's services.
 *
 * Which services are offered, and their checkout labels, are on the
 * Settings tab; this is what the services are.
 */
function uc_carrier_services_form($form, &$form_state, $carrier) {
  $form_state['carrier'] = $carrier;
  $form_state['definitions_key'] = 'services';
  $base = 'admin/store/settings/quotes/carrier/' . $carrier['id'];
  $form['note'] = array('#markup' => _uc_carrier_definitions_note($carrier, 'services', t('services')));
  $form['help'] = array(
    '#markup' => '<p>' . t('The carrier\'s services. Choose which are offered at checkout, and their labels, on the <a href="@url">Settings</a> tab. Use the empty row to add a service, then add its prices on the Prices tab. "Size limits" is for a service that takes a size beyond the usual limits.', array('@url' => url($base))) . '</p>',
  );

  $form['services'] = array(
    '#theme' => 'uc_carrier_definitions_table',
    '#header' => array(t('Order'), t('Name'), t('Description'), t('Size limits'), t('Delete')),
    '#tree' => TRUE,
  );
  $rows = $carrier['services'] + array('_new' => array('title' => '', 'description' => '', 'formats' => array()));
  $i = 0;
  foreach ($rows as $id => $service) {
    $form['services'][$id] = array(
      'order' => array('#type' => 'textfield', '#title' => t('Order'), '#title_display' => 'invisible', '#default_value' => ++$i, '#size' => 2, '#attributes' => array('class' => array('uc-carrier-number'))),
      'title' => array(
        '#type' => 'textfield',
        '#title' => t('Name'),
        '#title_display' => 'invisible',
        '#default_value' => $service['title'],
        '#size' => 30,
        '#attributes' => $id == '_new' ? array('placeholder' => t('New service')) : array(),
      ),
      'description' => array(
        '#type' => 'textfield',
        '#title' => t('Description'),
        '#title_display' => 'invisible',
        '#default_value' => $service['description'],
        '#size' => 30,
      ),
      'limits' => array(
        '#markup' => $id == '_new' ? '' : l(empty($service['formats']) ? t('usual') : t('own limits'), $base . '/services/' . $id),
      ),
      'delete' => $id == '_new' ? array('#markup' => '') : array('#type' => 'checkbox', '#title' => t('Delete'), '#title_display' => 'invisible'),
    );
  }

  _uc_carrier_definitions_actions($form);
  return $form;
}

/**
 * Form submission handler for uc_carrier_services_form().
 */
function uc_carrier_services_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $rows = $form_state['values']['services'];
  uasort($rows, function ($a, $b) {
    return (float) $a['order'] <=> (float) $b['order'];
  });

  $services = array();
  foreach ($rows as $id => $row) {
    if (!empty($row['delete']) || trim($row['title']) === '') {
      continue;
    }
    $existing = $id != '_new' && isset($carrier['services'][$id]) ? $carrier['services'][$id] : array('enabled' => FALSE, 'formats' => array());
    if ($id == '_new') {
      $id = uc_carrier_machine_name($row['title'], $carrier['services'] + $services);
    }
    $services[$id] = array(
      'title' => trim($row['title']),
      'description' => trim($row['description']),
      'enabled' => $existing['enabled'],
      'formats' => $existing['formats'],
    );
  }
  uc_carrier_definitions_save($carrier['id'], 'services', $services);
  backdrop_set_message(t('Services saved.'));
  _uc_carrier_orphan_warning($carrier['id'], 'service', array_keys($services));
}

/**
 * Form builder: one service's own size limits.
 */
function uc_carrier_service_limits_form($form, &$form_state, $carrier, $service_id) {
  if (!isset($carrier['services'][$service_id])) {
    backdrop_not_found();
    backdrop_exit();
  }
  $form_state['carrier'] = $carrier;
  $form_state['service'] = $service_id;
  $service = $carrier['services'][$service_id];
  $base = 'admin/store/settings/quotes/carrier/' . $carrier['id'];
  backdrop_set_title(t('@service: size limits', array('@service' => $service['title'])));

  $form['back'] = array('#markup' => '<p>' . l(t('‹ Back to services'), $base . '/services') . '</p>');
  $form['help'] = array(
    '#markup' => '<p>' . t('Only where this service differs from the usual limits (shown greyed), e.g. Royal Mail Tracked takes large letters up to 1kg. Leave empty to use the usual limit.') . '</p>',
  );
  $form['formats'] = array(
    '#theme' => 'uc_carrier_definitions_table',
    '#header' => array(t('Size'), t('Length'), t('Width'), t('Depth'), t('Weight limit')),
    '#tree' => TRUE,
  );
  foreach ($carrier['formats'] as $id => $format) {
    $own = isset($service['formats'][$id]) ? $service['formats'][$id] : array();
    $form['formats'][$id]['title'] = array('#markup' => check_plain($format['title']));
    foreach (array('length' => t('mm'), 'width' => t('mm'), 'depth' => t('mm'), 'weight' => t('g')) as $key => $suffix) {
      $usual = $format[$key] ? rtrim(rtrim(number_format($format[$key], 1, '.', ''), '0'), '.') : '';
      $form['formats'][$id][$key] = _uc_carrier_number_cell(backdrop_ucfirst($key), isset($own[$key]) ? $own[$key] : '', $suffix, $usual);
    }
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['#attached']['css'][] = backdrop_get_path('module', 'uc_carrier') . '/css/uc_carrier.admin.css';
  return $form;
}

/**
 * Form validation handler for uc_carrier_service_limits_form().
 */
function uc_carrier_service_limits_form_validate($form, &$form_state) {
  _uc_carrier_validate_numbers($form_state['values']['formats'], array('formats'), array('length', 'width', 'depth', 'weight'));
}

/**
 * Form submission handler for uc_carrier_service_limits_form().
 *
 * Saving a service's limits makes the site's services its own.
 */
function uc_carrier_service_limits_form_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $own = array();
  foreach ($form_state['values']['formats'] as $id => $row) {
    foreach (array('length', 'width', 'depth', 'weight') as $key) {
      if (trim($row[$key]) !== '') {
        $own[$id][$key] = $key == 'weight' ? (int) $row[$key] : (float) $row[$key];
      }
    }
  }
  $services = $carrier['services'];
  $services[$form_state['service']]['formats'] = $own;
  uc_carrier_definitions_save($carrier['id'], 'services', $services);
  backdrop_set_message(t('Size limits saved.'));
  $form_state['redirect'] = 'admin/store/settings/quotes/carrier/' . $carrier['id'] . '/services';
}

/**
 * Submit handler: saves the zone names from the Zones tab.
 */
function uc_carrier_zone_names_submit($form, &$form_state) {
  $carrier = $form_state['carrier'];
  $zones = array();
  foreach ($form_state['values']['zones'] as $id => $row) {
    if (!empty($row['delete']) || trim($row['title']) === '') {
      continue;
    }
    if ($id == '_new') {
      $id = uc_carrier_machine_name($row['title'], $carrier['zones'] + $zones);
    }
    $zones[$id] = trim($row['title']);
  }
  uc_carrier_definitions_save($carrier['id'], 'zones', $zones);
  backdrop_set_message(t('Zone names saved.'));
  _uc_carrier_orphan_warning($carrier['id'], 'zone', array_keys($zones));
}

/**
 * Form builder: offers supplied price lists not yet imported or ignored.
 */
function uc_carrier_pending_form($form, &$form_state, $carrier) {
  $form_state['carrier'] = $carrier;
  foreach (uc_carrier_pending_price_lists($carrier) as $date => $list) {
    $form[$date] = array(
      '#type' => 'container',
      '#attributes' => array('class' => array('messages', 'warning')),
    );
    $form[$date]['text'] = array(
      '#markup' => t('A new price list is available: %title. It was supplied with a module update and takes over on @date if imported; prices you added for anything it does not cover are carried over, and a list of your own from that date is kept.', array(
        '%title' => $list['title'],
        '@date' => format_date($list['effective'], 'custom', 'j M Y'),
      )) . ' ',
    );
    $form[$date]['import'] = array(
      '#type' => 'submit',
      '#value' => t('Import'),
      '#name' => 'import_' . $date,
      '#list_date' => $date,
      '#list_import' => TRUE,
    );
    $form[$date]['ignore'] = array(
      '#type' => 'submit',
      '#value' => t('Ignore'),
      '#name' => 'ignore_' . $date,
      '#list_date' => $date,
      '#list_import' => FALSE,
    );
  }
  return $form;
}

/**
 * Form submission handler for uc_carrier_pending_form().
 */
function uc_carrier_pending_form_submit($form, &$form_state) {
  $button = $form_state['triggering_element'];
  $result = uc_carrier_handle_supplied_list($form_state['carrier'], $button['#list_date'], $button['#list_import']);
  if (!$button['#list_import']) {
    backdrop_set_message(t('Ignored. It can still be imported from Import price list.'));
  }
  elseif ($result && $result['errors']) {
    foreach (array_slice($result['errors'], 0, 10) as $error) {
      backdrop_set_message($error, 'error');
    }
  }
  elseif ($result && !empty($result['kept'])) {
    backdrop_set_message(t('You already have a price list from that date, so it was kept and the supplied one was not imported. To use the supplied one instead, choose it on Import price list.'), 'warning');
  }
  elseif ($result) {
    backdrop_set_message(t('Imported @count prices.', array('@count' => $result['count'])) . (!empty($result['carried']) ? ' ' . format_plural($result['carried'], '1 price of your own carried over.', '@count prices of your own carried over.') : ''));
  }
}
