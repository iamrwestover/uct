<?php
$keys = element_children($form);
$first_row = $form[array_shift($keys)];
$grid_max = 10;
$grid_width = array(
  'prod_id' => 3,
  'description' => 4,
  'qty' => 1,
  'price' => 1,
  'amt' => 1,
);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form" data-uk-nestable="{handleClass:'uk-nestable-handle', maxDepth: 1}">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase uk-text-center">
      <div class="uk-width-8-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-4-5">
            <div class="av-nestable-cell">

              <div class="uk-grid uk-grid-collapse">
                <div class="uk-width-1-10">

                  <div class="uk-grid uk-grid-collapse">
                    <div class="uk-width-1-2">&nbsp;</div>
                    <div class="uk-width-1-2 uk-text-left">#</div>
                  </div>

                </div>

                <div class="uk-width-9-10"><?php print $first_row['product_id']['#title']; ?></div>
              </div>

            </div>
          </div>
          <div class="uk-width-1-5">
            <div class="av-nestable-cell">
              <?php print $first_row['uom_id']['#title']; ?>
            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-2-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-5">
            <div class="av-nestable-cell">
              <?php print $first_row['qty']['#title']; ?>
            </div>
          </div>
          <div class="uk-width-2-5">
            <div class="av-nestable-cell">
              <?php print $first_row['price']['#title']; ?>
            </div>
          </div>
          <div class="uk-width-2-5">
            <div class="av-nestable-cell">
              <?php print $first_row['amount']['#title']; ?>
            </div>
          </div>
        </div>
      </div>

    </div>
  </div>

  <?php print drupal_render_children($form); ?>
</div>
