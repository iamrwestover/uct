<table>
  <tbody>
  <tr>
    <td>
      <strong>P.O. No.:</strong> <?php print $po_num; ?><br />
      <strong>Date:</strong> <?php print $transaction_date; ?><br />
    </td>
    <td class="uk-text-right">
      <strong>Ship To:</strong><br />
      <?php print $shipping_address; ?>
    </td>
  </tr>
  </tbody>
</table>

<table>
  <thead>
    <tr>
      <th class="uk-text-left">#</th>
      <th class="uk-text-left">Product</th>
      <th class="uk-text-left">UOM</th>
      <th class="uk-text-right">Qty.</th>
      <th class="uk-text-right">Unit Cost</th>
      <th class="uk-text-right">Total</th>
    </tr>
  </thead>
  <tbody>
    <?php $index = 0; ?>
    <?php foreach ($po->items as $item): ?>
      <?php $index++; ?>
      <tr>
        <td><?php print $index; ?></td>
        <td><?php print check_plain($item->product_title); ?></td>
        <td><?php print check_plain($item->uom_title); ?></td>
        <td class="uk-text-right"><?php print $item->qty; ?></td>
        <td class="uk-text-right"><?php print number_format($item->cost, 2); ?></td>
        <td class="uk-text-right"><?php print number_format($item->total, 2); ?></td>
      </tr>
    <?php endforeach; ?>
  </tbody>
  <tfoot>
    <tr>
      <td colspan="5" class="uk-text-right uk-text-bold">Subtotal</td>
      <td class="uk-text-right uk-text-bold"><?php print number_format($po->subtotal, 2); ?></td>
    </tr>
    <tr>
      <td colspan="4" class="uk-text-right">Discount:</td>
      <td class="uk-text-right"><?php print $po->discount_value_text; ?></td>
      <td class="uk-text-right">- <?php print number_format($po->discounted_value, 2); ?></td>
    </tr>
    <tr>
      <td colspan="5" class="uk-text-right"><h2>Total</h2></td>
      <td class="uk-text-right"><h2><?php print number_format($po->grand_total, 2); ?></h2></td>
    </tr>
  </tfoot>
</table>