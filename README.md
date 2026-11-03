# Ubercart Carrier Rates

Shipping quotes for Ubercart from carrier price lists. The price depends on
the parcel's weight, its size (letter, large letter, parcel...) and the
destination zone, as carriers such as Royal Mail actually charge.

- **Price lists you control.** Prices are entered in a grid (weight bands
  down, zones across), or imported from JSON or CSV. Each list has the
  date it takes effect, so next season's prices can be loaded before they
  start. You are warned when the prices in force may be out of date.
- **Carriers change; your site can keep up.** The services, sizes and
  zones come with the carrier module, but each site can edit, add or
  remove them without waiting for a module update, and reset to the
  module's defaults at any time.
- **Several parcels.** An order too big for one parcel is split, and each
  parcel is priced.
- **Packing settings per product.** "Ship as at least" a given size (a cap
  as a small parcel, not a flimsy large letter), and protective wrapping
  added once around the packed items.
- **Tracking.** Tracking numbers on the order page link to the carrier's
  tracking site; new shipments are filled in with the service the customer
  chose.

## Submodules

- **Royal Mail (UK)**: 1st and 2nd Class, Signed For, Tracked 24/48 (with
  and without signature), Special Delivery, and International Economy,
  Standard, Tracked and Tracked & Signed. Royal Mail's zones and sizes are
  built in, with Royal Mail's retail prices from 5 October 2026 as a
  starting point.
- **Carrier Rates Packaging**: your boxes, envelopes and bags, with their
  sizes, weights and stock. Quotes use the smallest packaging that fits;
  the shipment form records what was used and takes it off stock; low
  stock is flagged by e-mail. "Estimated" packaging covers whatever is to
  hand. A product analysis page shows what each product fits, and which
  products are missing a weight or size.

Planned: Parcelforce and DPD; shipping types (e.g. collection only,
pallet); carriers defined in the admin, such as a local pallet firm or
collection in person.

## Requirements

- Ubercart: Shipping Quotes (`uc_quote`) and Shipping (`uc_shipping`).

## Installation

1. Install this module using the official Backdrop CMS instructions at
   https://docs.backdropcms.org/documentation/extend-with-modules.
2. Enable **Carrier Rates** and a carrier, e.g. **Royal Mail (UK)**, and
   optionally **Carrier Rates Packaging**.
3. Enable the carrier at *Store > Configuration > Shipping quotes > Methods*, and
   configure it there (services offered, zones, prices).
4. Give your products a weight and dimensions. Products without a size are
   treated as small extras that fit alongside the rest.
5. Check the supplied prices against the carrier's: they are retail prices,
   and business or online postage often costs less.

## Keeping prices up to date

Prices supplied with a carrier module are correct on the date shown and
are **not updated by the module maintainers**: carriers change their prices
once or twice a year, and keeping several carriers' prices current is a
service, not part of the free module. Keep your prices up to date on the
Prices tab (a new list can be prepared ahead of each change), or subscribe
to automatic price updates when that service is available. The module warns
on the Prices tab and the Status report when the prices in force may be out
of date (for Royal Mail, after 7 months).

See [MANUAL.md](MANUAL.md) for the settings, packaging and the price list
format.

## Issues

Bugs and feature requests should be reported in the issue queue:
https://github.com/backdrop-contrib/uc_carrier/issues

## Security
Reviewed on 1 November 2026 for the kinds of vulnerability reported in the Drupal security advisories of 7 to 28 October 2026. Those advisories were for Drupal 10/11 and do not apply to Backdrop directly; this module was checked for the same classes of mistake.

Please report security issues privately to security@backdropcms.org, not in the issue queue.

## Current Maintainer(s)
- Steve Moorhouse (albanycomputers) (https://github.com/albanycomputers)
- Additional maintainers and contributors welcome.

---

## Credits
- Steve Moorhouse - Zulip (DrAlbany)
- Assisted by AI.

- Current development is sponsored by [Albany Computer Services](https://www.albany-computers.co.uk), providers of computer support, [web design](https://www.albanywebdesign.co.uk), and [web hosting](https://www.albany-hosting.co.uk).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory
for complete text.
