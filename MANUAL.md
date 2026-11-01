# Ubercart Carrier Rates manual

## Getting started

1. Enable **Carrier Rates** and a carrier (e.g. **Royal Mail (UK)**), and
   optionally **Carrier Rates Packaging**.
2. *Store > Settings > Shipping quotes > Methods*: enable the carrier, then
   **configure** next to it.
3. **Settings** tab: tick the services to offer at checkout.
4. **Prices** tab: check the supplied prices against what you pay.
5. Give products a weight and dimensions (the **Weight** and **Length /
   Width / Height** fields when editing a product).

Permission for all carrier pages: **Administer carrier rates**.

## How a quote is worked out

1. **Parcels.** By default the order is one parcel: the items stacked, as
   long and wide as the largest item and as deep as all of them together,
   plus the carrier's packaging weight. With the packaging submodule, the
   order goes in the smallest packaging that holds it, or is split across
   several (see Packaging).
2. **Sizes without a size.** Products with no dimensions use the carrier's
   default item size; with no default they count for weight only, as small
   extras (stickers, badges) do. If no product in the order has a size, the
   size is chosen **by weight alone**, so a light but bulky item may be
   quoted as a letter: give products a size.
3. **Protective wrapping** (per product) is added once on every side of the
   parcel.
4. **Size.** Each parcel gets the smallest of the carrier's sizes it fits,
   using the service's own limits where it has them (Royal Mail Tracked
   takes large letters up to 1kg). A product's **Ship as at least** rules
   out smaller sizes. If a service has no price for that size, the next
   larger size is tried, then prices for any size.
5. **Zone** from the delivery address's country (Zones tab). Postage always
   follows the delivery address, not the billing address.
6. **Price**: the lowest weight band the parcel fits, in the price list in
   force today. Several parcels are each priced and added together; the
   checkout label says how many ("1st Class (2 parcels)").
7. **Handling fee** is added once per order.

A service is offered only if it can take every parcel. If no service can
take the order, the fallback rate is offered, if set.

## Carrier pages

Tabs: **Settings | Services | Sizes | Zones | Prices**.

### Settings

- **Services offered** at checkout, and an optional checkout label for each
  (default: carrier name + service, e.g. "Royal Mail 1st Class").
- **Handling**: fixed fee and/or percentage, added to every quote.
- **Parcel**: packaging weight (when not using the packaging submodule) and
  a default item size for products without dimensions.
- **Fallback**: a rate offered when no service can take the order.
- **Tracking URL**, with `[tracking-number]` where the number goes.
- **Import new price lists supplied with module updates automatically**:
  off by default (see Prices).

### Services, Sizes and Zone names

These come with the carrier module. A site can change them when the carrier
changes its products, without waiting for a module update.

- **Services**: name, description and order; add with the empty row, or
  delete. **Size limits** (per service) is for a service that takes a size
  beyond the usual limits; leave a limit empty for the usual one.
- **Sizes**: smallest first, with length, width, depth (mm) and weight
  limit (g); leave a limit empty for none. The price grid headings show
  these limits in cm and kg.
- **Zone names** (on the Zones tab, collapsed): rename or add zones.

Each says whether the site has its own changes, and **Reset to module
defaults** goes back. Only your changes are stored: module updates can
still add new services, sizes or zones (and change ones you have not
edited), but never undo what you added, changed, deleted or reordered.
Renaming keeps the prices; deleting leaves them unused (with a warning),
and they come back if the item is added again with the same name. New
items get their prices on the Prices tab.

### Zones

The zone of each country installed in the store. Only countries you change
are stored, so a carrier update to its own zone list still reaches the
rest. Countries not installed in Ubercart cannot be chosen at checkout at
all; add them at *Store > Settings > Countries*.

### Prices

The price lists, by effective date: **In force**, **Scheduled** (starts
later) or **Superseded**, each with view, edit, export CSV, export JSON and
delete.

**Keeping prices up to date.** Prices supplied with a carrier module are
correct on the date shown and are not updated by the module maintainers.
Prepare a new list ahead of each carrier price change (**New price list**,
copy, edit), or subscribe to automatic price updates when available.
Warnings appear here and on the Status report:

- When the carrier is known to have changed its prices and this site has
  no list from that date: "Royal Mail changed its prices on 6 April 2027.
  The prices in force on this site are from 5 October 2026." Module
  updates keep the carrier's latest price change date current.
- Up to 60 days before an announced change: "Royal Mail prices change on
  6 April 2027. Prepare a new price list from that date."
- Otherwise, when the prices in force are older than the carrier's review
  period (Royal Mail: 7 months) and no newer list is scheduled.

**Price lists supplied with the module.** A carrier module comes with the
carrier's published prices at the time. All supplied lists are imported
when the carrier is first enabled. If a later module version includes a
new list, it is shown here and on the Status report as "A new price list is
available", with **Import** and **Ignore**; imported, it takes over on its
own date and your existing lists are not changed. Prices you added for
anything the supplied list does not cover (your own service, zone, size or
cover level) are carried over into it. If you already have a list of your
own from the same date, yours is kept and the supplied one is not imported
(choose it on **Import price list** to use it instead). Supplied lists are
retail prices: if you pay online or business prices, keep your own list
and ignore them. **Import ... automatically** on the Settings tab lets cron
import them instead.
Below, the list in force (or the one chosen with **view**) is shown one
service at a time: pick it from **Service**.

- **New price list**: enter the date the carrier's new prices start and
  copy the current list, then edit what changed. The current list stays in
  force until that date.
- **Edit**: pick the **Service**. Prices are grids, one per size, and one
  per cover level where a service has several (Special Delivery): weight
  bands down, zones across. Each heading shows the size's limits and the
  cover, e.g. "Small parcel - not exceeding: L: 45cm W: 35cm D: 16cm -
  Weight limit: 2kg - compensation up to £75.00".
  - Weights are the top of each band, in grams.
  - Leave a price empty where the service is not offered; clear a weight to
    remove a band; the empty row adds a band.
  - Only zones with prices are shown; **Show all zones** adds the rest.
  - **Add a size or compensation level** adds a grid. **Compensation
    (cover)** is the cover included at those prices: 75 shows as
    "compensation up to £75.00". For several cover levels, add one grid per
    level; the cheapest is quoted.
  - Switching service with unsaved changes asks first.
- Enter the carrier's own prices. To allow for packaging or handling, use
  the handling fee, packaging or a product's "Ship as at least", not the
  carrier's prices: a new price list replaces them from its date.
- **Import price list**: a supplied list (including one you ignored), or
  a JSON or CSV file (see below).

The carrier must also be enabled on the Methods page and, as for any
Ubercart method, it can be limited with the Rules condition on its quote.

## Product packing settings

On each product, under **Shipping settings > Packing**:

- **Ship as at least**: a size the product never goes smaller than, e.g. a
  cap as a small parcel rather than a flimsy large letter. The largest in
  the order applies to the whole order; packaging that would make a smaller
  size is not suggested.
- **Protective wrapping (mm per side)**: bubble wrap or padding around the
  packed items, added once on every side of the parcel, not to each item.
  Three 20 mm deep frames with 20 mm wrapping make a 100 mm deep parcel,
  not 120 mm. The thickest in a parcel applies.

## Packaging (submodule)

Enable **Carrier Rates Packaging**, then *Store > Settings > Shipping quotes
> Packaging*.

- **Add packaging** for each box, envelope or bag: inside size (what fits),
  outside size (what the carrier measures; empty = same as inside), weight
  of the packaging plus filler, the heaviest contents it holds, stock and
  the reorder level. Untick **In use** to stop using it without deleting it.
- **Estimated**: for whatever is to hand (e.g. boxes kept from deliveries):
  give a typical size and weight; stock is not counted and there is no
  reorder warning. Bought packaging, recycled or not, is ordinary stock.
- **Quotes** use the smallest packaging that holds the whole order,
  preferring packaging in stock but never refusing a quote over stock. If
  none holds it all, the order is split into several parcels. A single item
  may fill its packaging; several items may fill only the **fill factor**
  (default 80%) of the inside. The packaging's outside decides the
  carrier's size, and its weight is added to the items.
- **Order page**: a **Packing** pane on the admin order view shows the
  suggested packaging for each parcel, its stock and what goes in it, and
  once shipped, the packaging recorded.
- **Dispatch**: on the shipment form each package has **Packaging used**,
  with a suggestion. Saving takes it off stock; changing it, or deleting
  the shipment, puts it back. Split an order into several packages on the
  order's Packages tab.
- **Low stock**: when stock falls to the reorder level, a warning on screen
  and an e-mail (Settings on the Packaging page; default the site e-mail).
- **Product analysis** tab: the smallest packaging each product fits, the
  smallest in stock, and products with no weight or size, or that fit
  nothing.

## Royal Mail (UK)

- **Services**: 1st and 2nd Class; Signed For 1st and 2nd; Tracked 24 and
  48, with and without signature; Special Delivery by 1pm and 9am, with and
  without Saturday; International Economy (outside Europe only), Standard,
  Tracked (including Tracked Heavier, medium parcels to 20kg) and Tracked &
  Signed (documents only). 1st, 2nd and International Standard are offered
  by default.
- **Sizes**: letter, large letter, small parcel, medium parcel, with Royal
  Mail's UK limits. Tracked takes large letters up to 1kg.
- **Zones**: UK (including Isle of Man, Jersey, Guernsey), Europe Zones 1-3
  and World Zones 1-3 (USA is World Zone 3; unlisted countries World Zone
  1), from Royal Mail's October 2026 guide.
- **Prices**: Royal Mail's retail (stamp) prices from 5 October 2026.
  Online and business postage is often cheaper: enter what you pay.
- **Tracking** links to royalmail.com.

## Tracking links

On the order page (admin and customer), tracking numbers become links to the
carrier's tracking page when the shipment's method is the carrier, or its
Carrier text matches the carrier's name. When a shipment is created for an
order quoted by a carrier, Carrier and Shipment options are filled in from
the customer's choice.

## Price list files (optional)

**JSON** is what **export JSON** produces and what carrier modules supply:
the effective date and title, then one block per service, size and cover,
listing the zones it applies to and the weight bands (grams) with prices.

    {
        "carrier": "royalmail",
        "effective": "2026-10-05",
        "title": "Royal Mail retail prices from 5 October 2026",
        "source": "...",
        "prices": [
            { "service": "first_class", "zones": ["uk"], "format": "letter", "compensation": 20, "bands": { "100": 1.80 } },
            ...
        ]
    }

To supply a new list with a carrier module, edit it in the grid, export
JSON and put the file in the module's `prices` folder.

**CSV**: header row, then one row per price:

| Column | |
|---|---|
| service | Service machine name (listed on the import page) |
| zone | Zone machine name |
| format | Size machine name, or empty for a service priced by weight alone (e.g. Special Delivery) |
| max_weight | Top of the weight band, in grams |
| price | Price in the store currency |
| compensation | Optional: the cover included, shown in the grid heading |

    service,zone,format,max_weight,price,compensation
    first_class,uk,letter,100,1.80,20
    first_class,uk,large_letter,100,3.30,20
    first_class,uk,large_letter,250,3.60,20

Where a service has several compensation levels for the same band, the
cheapest is quoted. An import is all or nothing: any unknown code or bad
number and nothing is imported. Importing again for the same effective date
replaces that list. Export a list to see the format.

## Known limitations

- Packing is a sensible rule of thumb, not optimal box packing; orders of
  more than 200 items fall back to the stacked estimate.
- A product's "Ship as at least" applies to every parcel of the order.
- Sizes are per carrier, not per zone: Royal Mail's international small
  parcel (L+W+D up to 90cm, no side over 60cm) is not modelled, so some
  overseas items are quoted as medium parcels.
- Without the packaging submodule, an order is always one parcel.

## Developers

- `hook_uc_carrier_info()` defines a carrier: zones, sizes ("formats"),
  default country zones, services (with optional per-service size limits),
  tracking URL, an optional folder of JSON price lists (`price_lists`), and
  the price review period and wording (`price_review_months`,
  `price_changes`).
- `latest_price_change` in `hook_uc_carrier_info()`: the carrier's latest
  (or next announced) price change; bump it in a release.
- `uc_carrier.settings: price_changes` (carrier => Y-m-d): the same, set
  without a release (e.g. by a price notice service); the later date wins.
- `uc_carrier.settings: subscription_url`: once set, price warnings and
  the Prices tab offer the automatic price update subscription. See `uc_carrier.api.php` and
  `modules/uc_carrier_royalmail`.
- `hook_uc_carrier_info_alter()` changes carriers; a site's own services,
  sizes and zones are applied after it.
- `hook_uc_carrier_packages_alter()` replaces or splits the parcels for a
  quote (the packaging submodule uses it).
- Call `uc_carrier_delete_carrier('ID')` from a carrier's
  `hook_uninstall()`.
- Admin paths are kept within Backdrop's 9-part menu limit.
- After updating the module, run update.php (schema updates).
