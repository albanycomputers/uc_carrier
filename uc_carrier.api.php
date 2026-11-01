<?php
/**
 * @file
 * Hooks provided by the Carrier Rates module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Defines carriers.
 *
 * Each carrier becomes an Ubercart shipping method ("carrier_ID") whose
 * options are its enabled services. Prices come from price lists imported on
 * the carrier's Prices tab; the module only describes the carrier.
 *
 * Weights are in grams, dimensions in millimetres.
 *
 * @return array
 *   Carriers keyed by machine name (max 32 characters), each with:
 *   - title: The carrier name. Also matched against a shipment's Carrier text
 *     to link tracking numbers on shipments entered by hand.
 *   - description: (optional) Shown on the settings page and methods list.
 *   - tracking_url: (optional) Tracking page, with [tracking-number] as the
 *     placeholder.
 *   - formats: (optional) Size formats, smallest first, each an array of
 *     title and the limits length, width, depth and weight (0 = no limit).
 *     The parcel gets the first format it fits; a service without a price for
 *     that format is tried with the next. Omit for carriers priced by weight
 *     alone, and leave the CSV format column empty.
 *   - zones: Destination zone titles, keyed by machine name.
 *   - countries: (optional) Default zone per ISO 3166-1 alpha-2 code.
 *   - default_zone: (optional) Zone for countries not listed; empty means
 *     they are not quoted.
 *   - services: Each an array of title, optional description, enabled
 *     (offered by default on a new site), and optional formats: limits that
 *     differ for this service, keyed by format, e.g.
 *     array('large_letter' => array('weight' => 1000)).
 *   - price_review_months: (optional) After how many months the prices in
 *     force are flagged as possibly out of date. Default 12.
 *   - price_changes: (optional) When the carrier usually changes its prices,
 *     shown in that warning, e.g. "Royal Mail usually changes its prices in
 *     April and October."
 *   - latest_price_change: (optional) Date (Y-m-d) of the carrier's latest
 *     price change, or the next announced one. Bump it in a release: sites
 *     with no price list from that date are told plainly that the carrier
 *     has changed its prices (or will, within 60 days). Sites can also set
 *     uc_carrier.settings: price_changes.<carrier>; the later date wins.
 *   - price_lists: (optional) Folder of JSON price lists supplied with the
 *     module, one per price change, each with its own effective date (export
 *     one from the Prices tab). All are imported when the module is
 *     enabled; lists added by a later release are offered on the Prices tab,
 *     or imported by cron if the site chose to.
 */
function hook_uc_carrier_info() {
  $carriers['example'] = array(
    'title' => t('Example Couriers'),
    'tracking_url' => 'https://track.example.com/?ref=[tracking-number]',
    'zones' => array(
      'mainland' => t('UK mainland'),
      'highlands' => t('Highlands and islands'),
    ),
    'countries' => array('GB' => 'mainland'),
    'services' => array(
      'next_day' => array('title' => t('Next day'), 'enabled' => TRUE),
    ),
    'price_lists' => backdrop_get_path('module', 'example') . '/prices',
  );

  return $carriers;
}

/**
 * Alters carrier definitions.
 *
 * @param array $carriers
 *   The carriers, as returned by hook_uc_carrier_info() with defaults added.
 */
function hook_uc_carrier_info_alter(array &$carriers) {
  $carriers['royalmail']['services']['tracked_24']['enabled'] = TRUE;
}

/**
 * Alters the parcels worked out for a quote.
 *
 * By default the order is one parcel: the items stacked, plus the carrier's
 * packaging weight. A module can choose real packaging, or split the order
 * into several parcels; each is priced and the prices are added up. A
 * service that cannot take every parcel is not offered.
 *
 * @param array $packages
 *   A list of parcels, each with:
 *   - weight: Grams, including packaging.
 *   - dims: array(length, width, depth) in mm, longest first, or NULL when
 *     the size is unknown.
 *   Other keys may be added for the module's own use.
 * @param array $context
 *   - products: The order's shippable products.
 *   - carrier: The carrier.
 *   - settings: The carrier's site settings.
 */
function hook_uc_carrier_packages_alter(array &$packages, array $context) {
  // Ship each product line in its own 400 x 300 x 200 mm box of 300 g.
  $packages = array();
  foreach ($context['products'] as $product) {
    $weight = 300 + $product->qty * $product->weight * uc_weight_conversion($product->weight_units, 'g');
    $packages[] = array('weight' => (int) ceil($weight), 'dims' => array(400, 300, 200));
  }
}

/**
 * @} End of "addtogroup hooks".
 */
