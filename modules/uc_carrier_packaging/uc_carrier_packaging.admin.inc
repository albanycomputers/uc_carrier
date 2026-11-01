<?php
/**
 * @file
 * Admin pages for packaging: list, edit, settings and product analysis.
 */

/**
 * Formats inside or outside dimensions for display.
 */
function _uc_carrier_packaging_dims(array $dims) {
  return implode(' x ', array_map(function ($v) {
    return rtrim(rtrim(number_format($v, 1, '.', ''), '0'), '.');
  }, $dims)) . ' mm';
}

/**
 * Page callback: the packaging list, with settings below.
 */
function uc_carrier_packaging_list_page() {
  $base = 'admin/store/settings/quotes/packaging';
  $rows = array();
  $low = array();
  foreach (uc_carrier_packaging_all() as $box) {
    if ($box->estimated) {
      $stock = t('Estimated');
    }
    else {
      $stock = $box->stock;
      if ($box->status && $box->stock <= $box->reorder_level) {
        $stock = '<strong>' . $stock . '</strong> ' . t('(reorder)');
        $low[] = check_plain($box->name);
      }
    }
    $rows[] = array(
      'data' => array(
        check_plain($box->name),
        _uc_carrier_packaging_dims($box->inner),
        _uc_carrier_packaging_dims($box->outer),
        uc_weight_format($box->weight, 'g'),
        $box->max_weight ? uc_weight_format($box->max_weight, 'g') : '-',
        $stock,
        $box->estimated ? '-' : $box->reorder_level,
        $box->status ? t('In use') : t('Not in use'),
        l(t('edit'), $base . '/' . $box->pid) . ' | ' . l(t('delete'), $base . '/' . $box->pid . '/delete'),
      ),
      'class' => $box->status ? array() : array('warning'),
    );
  }

  if ($low) {
    backdrop_set_message(t('Time to order more: !list.', array('!list' => implode(', ', $low))), 'warning', FALSE);
  }

  $build['help'] = array(
    '#markup' => '<p>' . t('Quotes use the smallest packaging that holds the order, preferring packaging in stock, and split the order into several parcels when nothing holds it all. The packer confirms what was used on the shipment form, which takes it off stock. Packaging marked <em>Estimated</em> is whatever is to hand: a typical size and weight, stock not counted.') . '</p>',
  );
  $build['list'] = array(
    '#theme' => 'table',
    '#header' => array(t('Name'), t('Inside'), t('Outside'), t('Weight'), t('Holds up to'), t('Stock'), t('Reorder at'), t('Status'), t('Operations')),
    '#rows' => $rows,
    '#empty' => t('No packaging yet. Add the boxes, envelopes and bags you post in.'),
  );
  $build['settings'] = backdrop_get_form('uc_carrier_packaging_settings_form');

  return $build;
}

/**
 * Form builder: packaging settings.
 */
function uc_carrier_packaging_settings_form($form, &$form_state) {
  $config = config('uc_carrier_packaging.settings');
  $form['#config'] = 'uc_carrier_packaging.settings';

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => TRUE,
  );
  $form['settings']['fill_factor'] = array(
    '#type' => 'number',
    '#title' => t('Fill factor'),
    '#min' => 10,
    '#max' => 100,
    '#field_suffix' => '%',
    '#default_value' => $config->get('fill_factor'),
    '#description' => t('How much of the inside volume several items may fill, allowing for wasted space. A single item may fill its packaging completely.'),
  );
  $form['settings']['notify_email'] = array(
    '#type' => 'email',
    '#title' => t('Low stock e-mail'),
    '#default_value' => $config->get('notify_email'),
    '#description' => t('Told when packaging falls to its reorder level. Leave empty for the site e-mail address.'),
  );

  return system_settings_form($form);
}

/**
 * Form builder: adds or edits a packaging type.
 */
function uc_carrier_packaging_edit_form($form, &$form_state, $box = NULL) {
  $form_state['box'] = $box;
  $value = function ($key, $default = '') use ($box) {
    if (!$box) {
      return $default;
    }
    $v = $box->$key;
    return is_numeric($v) && strpos($v, '.') !== FALSE ? rtrim(rtrim($v, '0'), '.') : $v;
  };

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#required' => TRUE,
    '#default_value' => $value('name'),
    '#description' => t('As the packer knows it, e.g. "DL envelope", "Large letter mailer", "Box 300 x 300 x 60".'),
  );

  foreach (array('inner' => t('Inside dimensions'), 'outer' => t('Outside dimensions')) as $side => $title) {
    $form[$side] = array(
      '#type' => 'fieldset',
      '#title' => $title,
      '#attributes' => array('class' => array('container-inline')),
      '#description' => $side == 'inner' ? t('What fits inside, in mm.') : t('What the carrier measures, in mm. Leave empty to use the inside dimensions (fine for envelopes and bags).'),
    );
    foreach (array('length' => t('Length'), 'width' => t('Width'), 'depth' => t('Depth')) as $dim => $dim_title) {
      $form[$side][$side . '_' . $dim] = array(
        '#type' => 'number',
        '#title' => $dim_title,
        '#min' => 0,
        '#step' => 0.1,
        '#size' => 6,
        '#required' => $side == 'inner',
        '#default_value' => $value($side . '_' . $dim, $side == 'inner' ? '' : ''),
      );
    }
  }

  $form['weight'] = array(
    '#type' => 'number',
    '#title' => t('Weight'),
    '#min' => 0,
    '#field_suffix' => t('g'),
    '#default_value' => $value('weight', 0),
    '#description' => t('The packaging itself plus any filler, tape and labels.'),
  );
  $form['max_weight'] = array(
    '#type' => 'number',
    '#title' => t('Holds up to'),
    '#min' => 0,
    '#field_suffix' => t('g'),
    '#default_value' => $value('max_weight', 0),
    '#description' => t('Heaviest contents it can take; 0 for no limit.'),
  );

  $form['estimated'] = array(
    '#type' => 'checkbox',
    '#title' => t('Estimated'),
    '#default_value' => $value('estimated', 0),
    '#description' => t('Whatever is to hand, e.g. boxes kept from deliveries: enter a typical size and weight. Stock is not counted and there is no reorder warning.'),
  );
  $form['stock'] = array(
    '#type' => 'number',
    '#title' => t('In stock'),
    '#default_value' => $value('stock', 0),
    '#states' => array('invisible' => array(':input[name="estimated"]' => array('checked' => TRUE))),
  );
  $form['reorder_level'] = array(
    '#type' => 'number',
    '#title' => t('Reorder at'),
    '#min' => 0,
    '#default_value' => $value('reorder_level', 0),
    '#description' => t('Warn when stock falls to this number.'),
    '#states' => array('invisible' => array(':input[name="estimated"]' => array('checked' => TRUE))),
  );
  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('In use'),
    '#default_value' => $value('status', 1),
    '#description' => t('Packaging not in use is never chosen for quotes.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Form validation handler for uc_carrier_packaging_edit_form().
 */
function uc_carrier_packaging_edit_form_validate($form, &$form_state) {
  $v = $form_state['values'];
  $outer = array_filter(array($v['outer_length'], $v['outer_width'], $v['outer_depth']), 'strlen');
  if ($outer && count($outer) < 3) {
    form_set_error('outer', t('Enter all three outside dimensions, or none.'));
  }
  if (count($outer) == 3) {
    $inner = array($v['inner_length'], $v['inner_width'], $v['inner_depth']);
    $outer = array($v['outer_length'], $v['outer_width'], $v['outer_depth']);
    rsort($inner);
    rsort($outer);
    foreach ($inner as $i => $size) {
      if ($outer[$i] < $size) {
        form_set_error('outer', t('The outside cannot be smaller than the inside.'));
        break;
      }
    }
  }
}

/**
 * Form submission handler for uc_carrier_packaging_edit_form().
 */
function uc_carrier_packaging_edit_form_submit($form, &$form_state) {
  $v = $form_state['values'];
  $fields = array(
    'name' => trim($v['name']),
    'weight' => (int) $v['weight'],
    'max_weight' => (int) $v['max_weight'],
    'estimated' => (int) $v['estimated'],
    'stock' => (int) $v['stock'],
    'reorder_level' => (int) $v['reorder_level'],
    'status' => (int) $v['status'],
  );
  foreach (array('inner', 'outer') as $side) {
    foreach (array('length', 'width', 'depth') as $dim) {
      $fields[$side . '_' . $dim] = (float) $v[$side . '_' . $dim];
    }
  }

  if ($form_state['box']) {
    db_update('uc_carrier_packaging')->fields($fields)->condition('pid', $form_state['box']->pid)->execute();
  }
  else {
    db_insert('uc_carrier_packaging')->fields($fields)->execute();
  }
  backdrop_static_reset('uc_carrier_packaging_all');

  backdrop_set_message(t('Saved %name.', array('%name' => $fields['name'])));
  $form_state['redirect'] = 'admin/store/settings/quotes/packaging';
}

/**
 * Form builder: confirms deleting a packaging type.
 */
function uc_carrier_packaging_delete_form($form, &$form_state, $box) {
  $form_state['box'] = $box;
  $used = db_query('SELECT COUNT(*) FROM {uc_carrier_packaging_usage} WHERE pid = :pid', array(':pid' => $box->pid))->fetchField();

  return confirm_form(
    $form,
    t('Delete %name?', array('%name' => $box->name)),
    'admin/store/settings/quotes/packaging',
    $used ? t('It is recorded on @count shipped packages; those records will be removed. To keep them, untick "In use" instead.', array('@count' => $used)) : t('This cannot be undone.'),
    t('Delete')
  );
}

/**
 * Form submission handler for uc_carrier_packaging_delete_form().
 */
function uc_carrier_packaging_delete_form_submit($form, &$form_state) {
  $box = $form_state['box'];
  db_delete('uc_carrier_packaging_usage')->condition('pid', $box->pid)->execute();
  db_delete('uc_carrier_packaging')->condition('pid', $box->pid)->execute();
  backdrop_static_reset('uc_carrier_packaging_all');
  backdrop_set_message(t('Deleted %name.', array('%name' => $box->name)));
  $form_state['redirect'] = 'admin/store/settings/quotes/packaging';
}

/**
 * Page callback: the smallest packaging for each shippable product.
 */
function uc_carrier_packaging_analysis_page() {
  $boxes = uc_carrier_packaging_all(TRUE);
  $rows = array();
  $problems = 0;

  $result = db_query('SELECT n.nid, n.title, p.weight, p.weight_units, p.length, p.width, p.height, p.length_units FROM {uc_products} p INNER JOIN {node} n ON n.vid = p.vid WHERE p.shippable = 1 AND n.status = 1 ORDER BY n.title');
  foreach ($result as $product) {
    $product->qty = 1;
    $notes = array();
    if (!$product->weight) {
      $notes[] = t('No weight');
    }
    $items = uc_carrier_packaging_items(array($product));
    $item = $items[0];
    $smallest = $in_stock = '-';
    if (!$item['dims']) {
      $notes[] = t('No size: cannot check what it fits');
    }
    elseif ($boxes) {
      $any = FALSE;
      foreach ($boxes as $box) {
        if (uc_carrier_packaging_items_fit(array($item), $box)) {
          if (!$any) {
            $any = $box;
          }
          if (uc_carrier_packaging_available($box)) {
            $in_stock = check_plain($box->name);
            break;
          }
        }
      }
      if ($any) {
        $smallest = check_plain($any->name);
        if ($in_stock == '-') {
          $notes[] = t('None in stock that fits');
        }
      }
      else {
        $notes[] = t('Fits no packaging: needs at least @size', array('@size' => _uc_carrier_packaging_dims($item['dims'])));
      }
    }
    if ($notes) {
      $problems++;
    }
    $rows[] = array(
      l($product->title, 'node/' . $product->nid . '/edit'),
      $product->weight ? uc_weight_format($item['weight'], 'g') : '-',
      $item['dims'] ? _uc_carrier_packaging_dims($item['dims']) : '-',
      $smallest,
      $in_stock,
      implode('; ', $notes),
    );
  }

  $build['help'] = array(
    '#markup' => '<p>' . t('The smallest packaging each product fits on its own, and the smallest of those in stock. Products need a weight and a size for quotes and packing to be right.') . ' ' . format_plural($problems, '1 product needs attention.', '@count products need attention.') . '</p>',
  );
  if (!$boxes) {
    $build['none'] = array('#markup' => '<p>' . t('No packaging in use yet.') . '</p>');
  }
  $build['table'] = array(
    '#theme' => 'table',
    '#header' => array(t('Product'), t('Weight'), t('Size'), t('Smallest packaging'), t('Smallest in stock'), t('Notes')),
    '#rows' => $rows,
    '#empty' => t('No shippable products.'),
  );

  return $build;
}
