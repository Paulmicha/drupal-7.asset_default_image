drupal-7.asset_default_image
============================

Drupal 7 module - specific set of constraints related to the use of default image for asset entity reference fields.

- Upon deletion of any image asset, make sure its previous references
- are replaced by the default.
- Prevents the default visual from being deleted
- Defines a constant holding the asset ID of this default image.
- Supports multiple default images using commas as separator.

Caveats : 
- Requires manual setup of the default value for every image assets
entity reference fields.
- Likely incompatible with this patch : https://www.drupal.org/node/1368386

Possible evolutions :
- Set automatically the default value to all image assets entity reference fields at once
- Allow / disallow for a configurable set of fields and/or content types
