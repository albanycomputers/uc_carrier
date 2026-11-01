# Changelog

## 1.x-1.0.0-beta1 (unreleased)

- New: Carrier Rates. Shipping quotes from carrier price lists by weight
  band, size format and destination zone; price lists imported from CSV with
  an effective date, entered in a price grid or imported from CSV; per-site services, labels, zones, handling fee,
  packaging weight, default item size and fallback rate.
- New: tracking numbers on the order page link to the carrier; new shipments
  are pre-filled with the carrier and service the customer chose.
- New: an order can be quoted as several parcels; each is priced and the
  prices added (hook_uc_carrier_packages_alter).
- New submodule: Carrier Rates Packaging. Packaging stock (inside/outside
  size, weight, stock, reorder level, "Estimated" for whatever is to hand);
  quotes use the smallest packaging that fits or split the order; the
  shipment form records the packaging used per package and adjusts stock;
  low-stock warning and e-mail; product analysis page; a Packing pane on
  the admin order view suggesting what to pack the order in.
- New: a service can change a size's limits (hook_uc_carrier_info services
  'formats'); Royal Mail Tracked takes large letters up to 1kg.
- New: Services, Sizes and Zone names can be edited per site (with reset to
  the module's defaults), so a carrier's product changes need no module
  update.
- New: per-product packing settings: "Ship as at least" a size, and
  protective wrapping added once around the packed items.
- New: JSON price lists (import and export). Carrier modules supply their
  price lists as JSON; lists added by a module update are offered on the
  Prices tab and the Status report, or imported automatically by cron if
  the site chooses.
- New: warnings on the Prices tab and Status report when the carrier has
  changed its prices (or will within 60 days) and the site has no list from
  that date, or when the prices in force are old (Royal Mail: 7 months).
  Supplied prices are a dated starting point, not maintained.
- Module updates never overwrite a site's changes: services, sizes and zone
  names store only the site's differences, so new module items still
  arrive; a supplied price list never replaces a site list from the same
  date, and carries over prices the site added for anything it does not
  cover.
- New submodule: Royal Mail (UK), with UK and international services, zones,
  size formats and the retail price list from 5 October 2026.
