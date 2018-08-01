<?php
module_load_include('inc', 'avtxns', 'inc/account_chart.helpers');
//module_load_include('inc', 'avtxns', 'inc/po.helpers');
//module_load_include('inc', 'avtxns', 'inc/so.helpers');
//module_load_include('inc', 'avtxns', 'inc/gr/gr.helpers');
//module_load_include('inc', 'avtxns', 'inc/delivery/delivery.helpers');
//module_load_include('inc', 'avtxns', 'inc/ret/ret.helpers');
//module_load_include('inc', 'avtxns', 'inc/rept/rept.helpers');
//module_load_include('inc', 'avtxns', 'inc/sinv/sinv.helpers');
//module_load_include('inc', 'avtxns', 'inc/spay/spay.helpers');

/**
 * Load transaction details.
 * @param $id
 *
 * @return object
 */
function avtxns_txn_load($id, $refresh = FALSE) {
  if (empty($id) || $id == 'new') {
    return FALSE;
  }

  $id = (int) $id;
  if (empty($id)) {
    return NULL;
  }

  $data = &drupal_static('avbase', array());
  $txns = &$data['txns'];
  if (isset($txns[$id]) && !$refresh) {
    return $txns[$id];
  }

  $conditions = array();
  $conditions[] = array('avtxns.id', $id);
  $rows = avtxns_txn_query_load($conditions, NULL, NULL, $refresh);
  $row = empty($rows[$id]) ? FALSE : $rows[$id];

  // Update static variable.
  $txns[$id] = $row;
  return $row;
}

/**
 * Load transaction details by ids.
 * @param array $ids
 *
 * @return array
 */
function avtxns_txn_load_ids($ids, $refresh = FALSE) {
  if (empty($ids)) {
    return FALSE;
  }

  $data = &drupal_static('avbase', array());
  $txns = &$data['txns'];

  $return = array();

  $query_ids = array();
  foreach ($ids as $id) {
    if (!empty($id)) {
      if (isset($txns[$id])) {
        $return[$id] = $txns[$id];
      }
      else {
        $query_ids[] = $id;
      }
    }
  }

  if (!empty($query_ids)) {
    $conditions = array();
    $conditions[] = array('avtxns.id', $query_ids, 'IN');
    $rows = avtxns_txn_query_load($conditions, NULL, NULL, $refresh);
    if (!empty($rows)) {
      $return += $rows;
    }
  }

  return $return;
}

/**
 * Advanced transaction load.
 * @param array $conditions
 *
 * @return array
 */
function avtxns_txn_query_load($conditions = array(), $offset = NULL, $limit = NULL, $refresh = FALSE) {
  //if (empty($conditions)) {
  //  return array();
  //}

  // Load products static variable.
  $avbase = &drupal_static('avbase', array());
  $avbase['txns'] = empty($avbase['txns']) ? array() : $avbase['txns'];
  $txns = &$avbase['txns'];

  // Set/load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  $_data_id = $conditions;
  sort($_data_id);
  $data_id = md5(json_encode($_data_id));
  if (isset($data[$data_id]) && !$refresh) {
    return $data[$data_id];
  }

  $query = db_select('avtbl_transactions', 'avtxns');
  $query->leftJoin('avtbl_clients', 'avclients', 'avclients.id = avtxns.client_id');
  $query->leftJoin('avtbl_categories', 'avcat_agent', "avcat_agent.id = avtxns.agent_id AND avcat_agent.group_id = CONCAT('agent', avclients.client_type)");
  $query->leftJoin('avtbl_categories', 'avcat_terms', 'avcat_terms.id = avtxns.term_id');
  $query->leftJoin('users', 'u', 'u.uid = avtxns.uid');
  $query->addField('avclients', 'display_name', 'client_name');
  $query->addField('avclients', 'address');
  $query->addField('avclients', 'city');
  $query->addField('avclients', 'province');
  $query->addField('avclients', 'zip_code');
  $query->addField('avclients', 'status', 'client_status');
  $query->addField('avclients', 'contact_number');
  $query->addField('avclients', 'credit_limit');
  $query->addField('avcat_agent', 'title', 'agent_name');
  $query->addField('avcat_terms', 'title', 'term_name');
  $query->addField('avcat_terms', 'data', 'term_data');
  $query->addField('u', 'name');
  $query->fields('avtxns');
  //->fields('avclient', array('display_name'))
  //->condition('avtxns.id', $id)
  //->execute();
  foreach ($conditions as $condition) {
    $field_name = $condition[0];
    $field_value = isset($condition[1]) ? $condition[1] : NULL;
    $field_operator = isset($condition[2]) ? $condition[2] : NULL;
    $query->condition($field_name, $field_value, $field_operator);
  }

  if (isset($offset) && isset($limit)) {
    $query->range($offset, $limit);
  }

  $query->orderBy('created', 'DESC');
  $rs = $query->execute();
  $rows = array();
  foreach ($rs as $row) {
    $ttd = avtxns_txn_types($row->transaction_type);
    $pending_txns_reference_type = empty($ttd['pending_txns_reference_type']) ? array() : (array) $ttd['pending_txns_reference_type'];
    $subtotal = 0;
    $credit_total = 0;
    $debit_total = 0;
    $journal_debit_total = $journal_credit_total = 0;
    if (!empty($row->id)) {
      $transaction_date_formatted = format_date($row->transaction_date, 'custom', 'Y M d');
      //$row->term_data = unserialize($row->term_data);
      $query = db_select('avtbl_transaction_details', 'avtxn_details');
      $query->fields('avtxn_details');

      // Accounts.
      if ($ttd['transaction'] == 'accounts') {
        $query->leftJoin('avtbl_account_chart', 'avachart', 'avachart.id = avtxn_details.account_id');
        $query->leftJoin('avtbl_clients', 'avclients', 'avclients.id = avtxn_details.client_id');
        $query->addField('avachart', 'title', 'account_name');
        $query->addField('avclients', 'display_name', 'client_name');
      }

      if (empty($ttd['non_product'])) {
        $query->leftJoin('avtbl_products', 'avprod', 'avprod.id = avtxn_details.item_id');
        $query->leftJoin('avtbl_categories', 'avcat', 'avcat.id = avtxn_details.uom_id');
        $query->leftJoin('avtbl_categories', 'avcat_base', 'avcat_base.id = avprod.uom_id');
        $query->leftJoin('avtbl_categories', 'avcat_catprod', 'avcat_catprod.id = avprod.category_id');
        //$query->leftJoin('avtbl_transaction_details', 'avtxn_reference', 'avtxn_reference.id = avtxn_details.ref_txn_dtl_id');
        //$query->leftJoin('avtbl_transactions', 'avtxn', 'avtxn.id = avtxn_reference.transaction_id');
        $query->addField('avprod', 'title', 'product_title');
        $query->addField('avprod', 'qty', 'onhand_qty');
        $query->addField('avprod', 'uom_id', 'base_uom_id');
        if ($row->status == AVTXNS_TXN_STATUS_PENDING) {
          $query->addField('avprod', 'discount1', 'prod_discount1');
          $query->addField('avprod', 'discount2', 'prod_discount2');
          $query->addField('avprod', 'discount3', 'prod_discount3');
          $query->addField('avprod', 'discount4', 'prod_discount4');
        }
        $query->addField('avcat', 'title', 'uom_title');
        $query->addField('avcat_base', 'title', 'base_uom_title');
        $query->addField('avcat_catprod', 'title', 'category_title');
        //$query->addField('avtxn_reference', 'transaction_id', 'ref_txn_id');
        //$query->addField('avtxn', 'transaction_type', 'ref_txn_type');

        //$query->orderBy('category_title', 'ASC');
        $query->orderBy('uom_title', 'DESC');
        $query->orderBy('id', 'ASC');
        //$query->orderBy('product_title', 'ASC');
      }
      $query->leftJoin('avtbl_transactions', 'avtxn', 'avtxn.id = avtxn_details.ref_txn_id');
      $query->addField('avtxn', 'changed', 'ref_txn_changed');
      $query->addField('avtxn', 'reference_id', 'ref_txn_ref_id');


      $query->condition('avtxn_details.transaction_id', $row->id)->execute();
      $details_rs = $query->execute();
      foreach ($details_rs as $item) {
        if (!isset($row->items_ref_txn_id) && !empty($item->ref_txn_id) && count($pending_txns_reference_type) == 1 && in_array($item->ref_txn_type, $pending_txns_reference_type)) {
          $row->items_ref_txn_id = $item->ref_txn_id;
          $row->items_ref_txn_type = $item->ref_txn_type;
          $row->items_ref_txn_ref_id = $item->ref_txn_ref_id;
        }

        // Capitalize item title.
        if (!empty($item->product_title)) {
          $item->product_title = strtoupper($item->product_title);
        }

        // Journal totals.
        if ($row->transaction_type == AVTXNS_TXN_TYPE_JOURNAL) {
          $journal_debit_value = empty($item->debit) ? 0 : $item->debit;
          $journal_credit_value = empty($item->credit) ? 0 : $item->credit;
          $journal_debit_total += (float) $journal_debit_value;
          $journal_credit_total += (float) $journal_credit_value;
        }

        $item->transaction_date_formatted = $transaction_date_formatted;
        $item->total = $item->cost * $item->qty;
        $discount = array();
        $prod_discount = array();
        $entered_discount = NULL;
        $existing_discount = NULL;
        for ($x = 1; $x <= 4; $x++) {
          $_discount = 0;
          $_max_discount = 0;
          $_discount_name = "discount$x";
          if (isset($item->$_discount_name)) {
            $_discount = round($item->$_discount_name, 2);
            $discount[] = $_discount;
          }

          $_prod_discount_name = "prod_discount$x";
          if (isset($item->$_prod_discount_name)) {
            $_max_discount = round($item->$_prod_discount_name * AVTXN_DISCOUNT_MULTIPLIER, 2);
            $prod_discount[] = $_max_discount;
          }
          //if ($row->status == AVTXNS_TXN_STATUS_PENDING && $_discount > $_max_discount) {
          //  $item->discount_over_limit = TRUE;
          //}

          $this_dc_val = (1 - ((float) $_discount / 100));
          $entered_discount = is_null($entered_discount) ? $this_dc_val : ($entered_discount * $this_dc_val);
          $this_dc_val = (1 - ((float) $_max_discount / 100));
          $existing_discount = is_null($existing_discount) ? $this_dc_val : ($existing_discount * $this_dc_val);
        }
        $discount = array_filter($discount);
        $prod_discount = array_filter($prod_discount);
        $existing_discount = round((float)(1 - $existing_discount), 4);
        $entered_discount = round((float)(1 - $entered_discount), 4);

        if ($row->status == AVTXNS_TXN_STATUS_PENDING && $entered_discount > $existing_discount) {
          $item->discount_over_limit = TRUE;
        }
        $item->discount_text = implode('/', $discount);
        $item->max_discount_text = empty($prod_discount) ? 0 : implode('/', $prod_discount);
        foreach ($discount as $v) {
          if (is_numeric($v)) {
            $v = (float)$v;
            $item->total = $item->total - (($item->total * $v) / 100);
          }
        }
        //$pending_return_type = empty($ttd['pending_return_type']) ? '-' : $ttd['pending_return_type'];
        //if (!empty($item->ref_txn_type)) {
        //  if ($item->ref_txn_type == $pending_return_type) {
        //    $credit_total += $item->total;
        //    $item->total *= -1;
        //  } else {
        //    $debit_total += $item->total;
        //  }
        //}

        $item->qty_per_uom = empty($item->qty_per_uom) ? 1 : $item->qty_per_uom;
        $item->incoming_base_qty = $item->qty * $item->qty_per_uom;
        //if ($row->transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN && empty($item->good_stock)) {
        //  $item->incoming_base_qty = 0;
        //}
        $item->remaining_base_qty = $item->incoming_base_qty;


        $credit_txn_types = empty($ttd['credit_txn_types']) ? array() : $ttd['credit_txn_types'];
        if (!empty($item->ref_txn_type) && in_array($item->ref_txn_type, $credit_txn_types)) {
          $row->credit_items[$item->id] = $item;
          $credit_total += $item->total;
        }
        else {
          $row->items[$item->id] = $item;
          $debit_total += $item->total;
          //if (!empty($ttd['total_is_entered_amount'])) {
          //  $debit_total += $item->paid_amount;
          //}
          //else {
          //  $debit_total += $item->total;
          //}
        }

        // Special fields for payment transactions.
        if (!empty($ttd['payment'])) {
          $invoice = avtxns_txn_load($item->ref_txn_id);
          if (!empty($invoice->id)) {
            // Get total paid - excluding payments before this transaction.
            //$query = "SELECT SUM(avtxn_details.cost) FROM {avtbl_transaction_details} avtxn_details
            //  INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_details.transaction_id
            //  WHERE avtxn_details.ref_txn_id = :invoice_id AND avtxn_details.transaction_id < :payment_id
            //     AND avtxn.status <> " . AVTXNS_TXN_STATUS_VOID;
            //$total_paid_for_invoice = db_query($query, array(':invoice_id' => $invoice->id, ':payment_id' => $row->id))->fetchField();
            //$total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
            //$item->balance = $invoice->grand_total - $total_paid_for_invoice;
            //$item->previous_payment = $total_paid_for_invoice;
            //$item->grand_total = $invoice->grand_total;

            $item->balance = isset($invoice->balance) ? $invoice->balance : 0;
            $item->previous_payment = isset($invoice->total_paid) ? $invoice->total_paid : 0;
            $item->grand_total = isset($invoice->grand_total) ? $invoice->grand_total : 0;
            if (isset($invoice->items_ref_txn_id)) {
              $item->items_ref_txn_id = $invoice->items_ref_txn_id;
              $item->items_ref_txn_type = $invoice->items_ref_txn_type;
            }
          }
        }

        //if (!empty($ttd['total_is_entered_amount'])) {
        //  $subtotal += $item->paid_amount;
        //}
        //else {
        //  $subtotal += $item->total;
        //}
      }
    }
    //$row->discount_value_text = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (round($row->discount_value, 2) . '%') : number_format($row->discount_value, 2);
    //$row->subtotal = $subtotal;
    //$row->discounted_value = $row->discount_type == AVBASE_DISCOUNT_PERCENT ? (($row->discount_value / 100) * $subtotal) : $row->discount_value;
    //$row->grand_total = $subtotal - $row->discounted_value;
    $row->grand_total = $debit_total - $credit_total;
    $row->credit_total = $credit_total;
    $row->debit_total = $debit_total;
    $row->journal_debit_total = $journal_debit_total;
    $row->journal_credit_total = $journal_credit_total;
    if (!empty($ttd['payment'])) {
      $row->amount_received = $row->grand_total + ((float) $row->amount_to_credit);
    }

    // Client address string.
    $row->address_string = avclients_address_string_format($row);

    // Attach received items.
    //avtxns_txn_attach_received_items($row);

    if (!empty($ttd['payable'])) {
      $query = "SELECT SUM(avtxn_details.cost) FROM {avtbl_transaction_details} avtxn_details
        INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_details.transaction_id
        WHERE avtxn_details.ref_txn_id = :invoice_id AND avtxn.status <> " . AVTXNS_TXN_STATUS_VOID;
      $total_paid_for_invoice = db_query($query, array(':invoice_id' => $row->id))->fetchField();
      $total_paid_for_invoice = empty($total_paid_for_invoice) ? 0 : $total_paid_for_invoice;
      $row->total_paid = $total_paid_for_invoice;
      $row->balance = round($row->grand_total - $total_paid_for_invoice, 2);
      //if ($row->status && (round($total_paid_for_invoice, 2) == round($row->grand_total, 2))) {
      //  // Close invoice.
      //  if (db_query("UPDATE {avtbl_transactions} SET status = 0 WHERE id = :invoice_id", array(':invoice_id' => $row->id))) {
      //    avbase_custom_watchdog(AVTXNS_TXN_TYPE_INVOICE . "__close", "ID: {$row->id}", array('#row' => $row, '#manner' => 'auto'));
      //    $row->status = 0;
      //  }
      //}
    }

    // Term data.
    if (isset($row->term_data)) {
      $row->term_data = unserialize($row->term_data);
    }

    $rows[$row->id] = $row;
  }

  // Update static variables.
  $data[$data_id] = $rows;
  $txns += $rows;
  return $rows;
}

/**
 * Loads transaction details and sends it to specified email.
 * If email is not specified, po email will be used.
 * @param $transaction_id
 * @param string $email
 *
 * @return bool
 */
function avtxns_send_txn_to_email($transaction_id, $email = '') {
  if (empty($transaction_id)) {
    return FALSE;
  }

  $transaction = avtxns_txn_load($transaction_id);
  $email = empty($email) ? $transaction->email : $email;
  if ($error = user_validate_mail($email)) {
    drupal_set_message($error, 'error');
    return FALSE;
  }

  //$transaction->discount_value = round($transaction->discount_value, 2);
  $params = array(
    'transaction' => $transaction,
    'transaction_id' => $transaction->id,
    'transaction_date' => format_date($transaction->created, 'custom', 'F d, Y'),
    'site_name' => variable_get('site_name', 'UCT'),
    'shipping_address' => variable_get('av_company_address', '-'),
    'client_name' => check_plain($transaction->client_name),
    //'payment_term_title' => check_plain($transaction->payment_term_title),
  );

  $params['transaction_table'] = theme('avtxns_txn_mail_table', $params);
  $message = drupal_mail('avtxns', 'transaction', $email, language_default(), $params);
  if (!empty($message['result'])) {
    drupal_set_message(t('Purchase order sent successfully to %mail.', array('%mail' => $email)));
    avbase_custom_watchdog("crud_mail_po", "Sent Purchase Order #@transaction_id to @client_name",
      array('@transaction_id' => $transaction->id, '@client_name' => $transaction->client_name , '#message' => $message)
    );
    return TRUE;
  }
  return FALSE;
}

/**
 * Advanced transaction load.
 * @param array $param_where
 * @param array $param_args
 * @param null $limit
 * @param null $offset
 * @param null $order_by
 * @param $count_query
 *
 * @return array
 */
function avtxns_txns_query_load($param_where = array(), $param_args = array(), $limit = NULL, $offset = NULL, $order_by = NULL, $more_select = array(), $more_joins = array(), $group_by = array(), $count_query = FALSE) {
  // Load vendors static variable.
  //$avbase = &drupal_static('avbase', array());
  //$avbase['vendors'] = empty($avbase['vendors']) ? array() : $avbase['vendors'];
  //$_vendors = &$avbase['vendors'];

  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());
  //$_data_id = $param_where;
  //sort($_data_id);

  // Set and check if data for data_id already exists.
  $func_args = func_get_args();
  foreach ($func_args as $func_k => $func_arg) {
    if (is_array($func_arg)) {
      asort($func_args[$func_k]);
    }
  }
  $data_id = md5(json_encode($func_args));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  //$transaction_types = empty($param_where['transaction_types']) ? array() : $param_where['transaction_types'];
  //unset($param_where['transaction_types']);

  //$queries = array();
  $select = array();
  if ($count_query) {
    $select[] = 'COUNT(avtxn.id)';
  }
  else {
    $select[] = 'avtxn.id';
    $select[] = 'avtxn.uid';
    $select[] = 'avtxn.status';
    $select[] = 'avtxn.transaction_type';
    $select[] = 'avtxn.transaction_date';
    //$select[] = 'avcat_agent.title AS agent_name';
    //$select[] = 'avclients.display_name';
    //$select[] = 'avtxn.term_id';
    //$select[] = 'avcat_term.title AS term_name';
    //$select[] = 'u.name';
    //$select[] = 'avtxn_detail.ref_txn_type';
    //$select[] = 'avtxn_detail.ref_txn_dtl_id';
    //$select[] = "IF (
    //    avtxn.transaction_type = '" . AVTXNS_TXN_TYPE_PAYMENT . "',
    //    SUM(avtxn_detail.paid_amount),
    //    (SUM(
    //        avtxn_detail.qty
    //        * avtxn_detail.cost
    //        * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    //        * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    //        * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    //        * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))
    //      )
    //    )
    //  ) AS grand_total";
  }
  $select = array_merge($select, $more_select);

  $joins = array();
  //$joins[] = 'LEFT JOIN {avtbl_clients} AS avclients ON avclients.id = avtxn.client_id';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON avcat_agent.id = avtxn.agent_id';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_term ON avcat_term.id = avtxn.term_id';
  //$joins[] = 'LEFT JOIN {users} AS u ON u.uid = avtxn.uid';
  $joins = array_merge($joins, $more_joins);
  //$joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.transaction_id = avtxn.id';
  //$joins[] = "LEFT JOIN {avtbl_transaction_details} AS avtxn_detail_ref ON
  //  IF (avtxn.transaction_type = '" . AVTXNS_TXN_TYPE_PAYMENT . "', 0, avtxn_detail_ref.id = avtxn_detail.ref_txn_dtl_id)";

  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_transactions} AS avtxn " . implode(' ', $joins);
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }

  if (empty($count_query)) {
    if (!empty($group_by)) {
      $query .= " GROUP BY " . implode(', ', $group_by);
    }
    //$query .= " GROUP BY avtxn.id, avtxn_detail.ref_txn_type";
    $order_by = is_null($order_by) ? array('avtxn.transaction_date DESC', 'avtxn.id DESC') : $order_by;
    if (!empty($order_by)) {
      $query .= " ORDER BY " . implode(', ', $order_by);
    }

    if (!is_null($limit)) {
      $query .= " LIMIT $limit";
      if (!is_null($offset)) {
        $query .= " OFFSET $offset";
      }
    }
  }


  if ($count_query) {
    $data[$data_id] = db_query($query, $param_args)->fetchField();
  }
  else {
    $data[$data_id] = db_query($query, $param_args);
    //$rows = $rs;
    //foreach ($rs as $row) {
    //  $rows[] = $row;
    //}
  }

  //$data[$data_id] = $rows;
  return $data[$data_id];
}

/**
 * Compute new product cost.
 * @param float $current_cost
 * @param $onhand_qty
 * @param float $new_cost
 * @param $incoming_base_qty
 *
 * @return float
 */
function avtxns_compute_product_cost($current_cost, $onhand_qty, $new_cost, $incoming_base_qty) {
  $current_cost = empty($current_cost) ? $new_cost : $current_cost;
  return ((($current_cost * $onhand_qty) + ($new_cost * $incoming_base_qty)) / ($onhand_qty + $incoming_base_qty));
}

/**
 * Compute new sales price by using the markup difference of current sales price
 * and current cost and adding it to the new cost.
 * @param float $current_sales_price
 * @param float $current_cost
 * @param float $new_cost
 *
 * @return float
 */
function avtxns_compute_sales_price($current_sales_price, $current_cost, $new_cost) {
  if ($current_sales_price <= $current_cost || empty($current_cost)) {
    $markup = 0.2;
  }
  else {
    $markup = ($current_sales_price - $current_cost) / $current_cost;
  }

  return array('markup' => $markup, 'price' => ($new_cost + ($new_cost * $markup)));
}

/**
 * Set common fields when purchasing items.
 *
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_purchase_fields(&$form, &$form_state, $txn_type, $row, $view_mode = FALSE){
  // Set parameters for client name field.
  $form['client_name']['#title'] = t('Vendor');
  $form['client_name']['#table_name'] = 'avtbl_vendors';
  $form['client_name']['#autocomplete_path'] = 'av/vendors/autocomplete';
  $form['client_name']['#avbase_autocomplete'] = array(
    'entity_group' => 'vendors'
  );
}

/**
 * Set common fields when selling items.
 *
 * @param $form
 * @param $form_state
 * @param $row
 */
function avtxns_sales_fields(&$form, &$form_state, $row, $view_mode = FALSE){
  // Set parameters for client name field.
  $form['client_name']['#title'] = t('Customer');
  $form['client_name']['#table_name'] = 'avtbl_customers';
  $form['client_name']['#autocomplete_path'] = 'av/customers/autocomplete';
  $form['client_name']['#avbase_autocomplete'] = array(
    'entity_group' => 'customers'
  );
}

/**
 * Ajax callback before submitting a Receive Items form.
 */
function avtxns_txn_form_ajax($form, $form_state) {
  $commands = array();
  if (form_get_errors()) {
    // Prevent modal from popping out.
    unset($form['cost_changes']['product_cost_changes']);
    unset($form['pending_orders']['selected_pos']);
  }

  if (!empty($form_state['redirect'])) {
    $commands[] = array(
      // Note: we will use this command in the .js file
      'command' => 'redirectUser',
      // The path the user is directed to is given here
      'path' => $form_state['redirect'],
    );
    return array('#type' => 'ajax', '#commands' => $commands);
  }
  else {
    return $form;
  }
}

/**
 * Check if product quantity is enough to give out.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_enough_qty(&$element, &$form_state) {
  $element_value = $element['#value'];
  if (is_numeric($element_value) == '') {
    return;
  }

  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (empty($item_row_key)) {
    return;
  }

  // Get form storage.
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];

  // Get item rows.
  $item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
  $element_item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  $item_id = empty($element_item_row['product_id']) ? NULL : (int)$element_item_row['product_id'];
  if (empty($item_id)) {
    form_error($element, t('Product not specified.'));
    return;
  }

  $element_parents = $element['#parents'];
  $total_product_qty = 0;
  foreach ($item_rows as $item_row_key => $item_row) {
    if (strtolower($item_row['product_title']) != strtolower($element_item_row['product_title'])) {
      continue;
    }

    // Get default qty if available.
    //$item_row_default_value = empty($default_item_rows[$item_row_key]['qty']['#default_value']) ? 0 : $default_item_rows[$item_row_key]['qty']['#default_value'];
    //$item_row_default_value = empty($element['#db_default_qty']) ? 0 : $element['#db_default_qty'];
    //$item_row_default_value = (int) $item_row_default_value;
    ////$item_row_default_qty_per_uom = empty($default_item_rows[$item_row_key]['qty_per_uom']['#default_value']) ? 1 : $default_item_rows[$item_row_key]['qty_per_uom']['#default_value'];
    //$item_row_default_qty_per_uom = empty($element['#db_def_qpu']) ? 1 : $element['#db_def_qpu'];
    //$item_row_default_qty_per_uom = (int) $item_row_default_qty_per_uom;
    //$item_row_default_base_qty = $item_row_default_value * $item_row_default_qty_per_uom;
    $item_row_default_base_qty = empty($item_row['def_bq']) ? 0 : $item_row['def_bq'];

    $qty_per_uom = empty($item_row['qty_per_uom']) ? 1 : (int)$item_row['qty_per_uom'];
    $qty = empty($item_row['qty']) ? 0 : (int)$item_row['qty'];
    $inputted_base_qty = $qty_per_uom * $qty;
    $total_product_qty += $inputted_base_qty - $item_row_default_base_qty;
  }

  $qty_details = avtxns_item_qty_details($item_id);
  $user_available_qty = (int) $qty_details['user_available'];
  // $user_available_qty = $user_available_qty < 0 ? 0 : $user_available_qty;
  if ($user_available_qty < $total_product_qty) {
    form_error($element, t('@verb @qty1 %product but remaining quantity is only @qty2.', array(
      '@qty1' => (empty($form_state['values']['id']) ? '' : 'additional ') . number_format($total_product_qty),
      '@qty2' => number_format($user_available_qty),
      '%product' => $element_item_row['product_title'],
      '@verb' => ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN ? 'Returning' : 'Requesting'),
    )));
  }

  //$item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  //$product_id = empty($item_row['product_id']) ? NULL : $item_row['product_id'];
  //$qty_per_uom = empty($item_row['qty_per_uom']) ? NULL : $item_row['qty_per_uom'];
  //if (!empty($qty_per_uom)) {
  //  $inputted_base_qty = $qty_per_uom * $element_value;
  //  //$current_qty = empty($product->qty) ? 0 : $product->qty;
  //
  //  $qty_details = avtxns_item_qty_details($product_id);
  //  $user_available_qty = (int) $qty_details['user_available'];
  //  if ($user_available_qty < $inputted_base_qty) {
  //    form_error($element, t('@verb @qty1 %product but available quantity is only @qty2.', array(
  //      '@qty1' => number_format($inputted_base_qty),
  //      '@qty2' => number_format($user_available_qty),
  //      '%product' => $item_row['product_title'],
  //      '@verb' => ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN ? 'Returning' : 'Requesting'),
  //    )));
  //  }
  //}
  //else {
  //  drupal_set_message('Validation against on-hand quantity failed.', 'warning');
  //}


}

/**
 * When editing transactions that increments quantity, make sure
 * that the edit doesn't result into a negative quantity.
 * @param $element
 * @param $form_state
 */
function avtxns_element_validate_resulting_qty(&$element, &$form_state) {
  $element_value = $element['#value'];
  if (is_numeric($element_value) == '') {
    return;
  }

  $item_row_key = isset($element['#item_row_key']) ? $element['#item_row_key'] : '';
  if (empty($item_row_key)) {
    return;
  }

  // Get form storage.
  $transaction_type = empty($form_state['values']['transaction_type']) ? '' : $form_state['values']['transaction_type'];

  // Get item rows.
  $item_rows = empty($form_state['values']['product_rows']) ? array() : $form_state['values']['product_rows'];
  $element_item_row = empty($item_rows[$item_row_key]) ? array() : $item_rows[$item_row_key];
  $item_id = empty($element_item_row['product_id']) ? NULL : (int)$element_item_row['product_id'];
  if (empty($item_id)) {
    form_error($element, t('Product not specified.'));
    return;
  }

  $element_parents = $element['#parents'];
  $total_product_qty = 0;
  foreach ($item_rows as $item_row_key => $item_row) {
    if (strtolower($item_row['product_title']) != strtolower($element_item_row['product_title'])) {
      continue;
    }

    // Get default qty if available.
    $item_row_default_base_qty = empty($item_row['def_bq']) ? 0 : $item_row['def_bq'];
    if (isset($element['#attributes']['data-column-name']) && $element['#attributes']['data-column-name'] == 'qty-diff') {
      // For Stock adjustment edit.
      $inputted_base_qty = $element_value;
    }
    else {
      // For other "qty_change = increment" transactions.
      $qty_per_uom = empty($item_row['qty_per_uom']) ? 1 : (int)$item_row['qty_per_uom'];
      $qty = empty($item_row['qty']) ? 0 : (int)$item_row['qty'];
      $inputted_base_qty = $qty_per_uom * $qty;
    }

    $total_product_qty += $inputted_base_qty - $item_row_default_base_qty;
  }

  $on_hand_qty = (int) avtxns_item_on_hand_qty($item_id);
  $resulting_qty = $on_hand_qty + $total_product_qty;
  if ($resulting_qty < 0) {
    form_error($element, t('Current on-hand quantity of %product is <strong>@onhand</strong>. Deducting @qty1 from current transaction value will end up with a negative quantity.', array(
      '@onhand' => number_format($on_hand_qty),
      '@qty1' => number_format(abs($total_product_qty)),
      '%product' => $element_item_row['product_title'],
    )));
  }

  // $qty_details = avtxns_item_qty_details($item_id);
  // $user_available_qty = (int) $qty_details['user_available'];
  // // $user_available_qty = $user_available_qty < 0 ? 0 : $user_available_qty;
  // if ($user_available_qty < $total_product_qty) {
  //   form_error($element, t('@verb @qty1 %product but available quantity is only @qty2.', array(
  //     '@qty1' => (empty($form_state['values']['id']) ? '' : 'additional ') . number_format($total_product_qty),
  //     '@qty2' => number_format($user_available_qty),
  //     '%product' => $element_item_row['product_title'],
  //     '@verb' => ($transaction_type == AVTXNS_TXN_TYPE_PURCHASE_RETURN ? 'Returning' : 'Requesting'),
  //   )));
  // }
}

/**
 * Return transaction type details.
 * @param $type
 *
 * @return mixed
 */
function avtxns_transaction_types($type) {
  $transaction_types = array(
    'po' => array(
      'title' => t('Purchase Order'),
      'base_path' => 'av/po',
      'id_prefix' => 'PO',
    ),
    'so' => array(
      'title' => t('Sales Order'),
      'base_path' => 'av/sales-order',
      'id_prefix' => 'SO',
    ),
    'gr' => array(
      'title' => t('Goods Received'),
      'base_path' => 'av/gr',
      'id_prefix' => 'GR',
    ),
    'dr' => array(
      'title' => t('Delivery'),
      'base_path' => 'av/delivery',
      'id_prefix' => 'DR',
    ),
    'ret' => array(
      'title' => t('Sales Return'),
      'base_path' => 'av/ret',
      'id_prefix' => 'RET',
    ),
    'rept' => array(
      'title' => t('Purchase Return'),
      'base_path' => 'av/rept',
      'id_prefix' => 'RET',
    ),
    'sinv' => array(
      'title' => t('Invoice'),
      'base_path' => 'av/sinv',
      'id_prefix' => 'INV',
    ),
    'spay' => array(
      'title' => t('Payment received'),
      'base_path' => 'av/spay',
      'id_prefix' => 'OR',
    ),
  );
  return $transaction_types[$type];
}

/**
 * Return return types.
 * @return array
 */
function avtxns_get_return_types() {
  return array(AVTXNS_RETURN_TYPE_RS => 'RPD', AVTXNS_RETURN_TYPE_RUD => 'RUD');
}

/**
 * Ajax validation if entered qty is less than on-hand qty.
 */
function avtxns_qty_check_and_reserve() {
  $data = array(
    //'post' => $_POST,
  );
  extract($_POST);

  $entered_item_id = (int) $_POST['entered_item_id'];
  $entered_qty = $_POST['entered_qty'];
  $transaction_id = (int) $_POST['transaction_id'];

  if (!empty($entered_item_id)) {
    $default_base_qty = 0;
    if (!empty($transaction_id)) {
      $transaction = avtxns_txn_load($transaction_id);
      if (!empty($transaction->id)) {
        $transaction_items = empty($transaction->items) ? array() : $transaction->items;
        foreach ($transaction_items as $ti) {
          if ($ti->item_id != $entered_item_id) {
            continue;
          }
          $default_qty = empty($ti->qty) ? 0 : $ti->qty;
          $default_qty_per_uom = empty($ti->qty_per_uom) ? 1 : $ti->qty_per_uom;
          $default_base_qty += ($default_qty * $default_qty_per_uom);
        }
      }
    }

    $qty_details = avtxns_item_qty_details($entered_item_id);
    $user_available = (int)$qty_details['user_available'];
    //$total_available = (int)$qty_details['total_available'];
    $entered_base_qty = is_numeric($entered_qty) ? $entered_qty : 0;
    if ($entered_base_qty <= $user_available) {
      avtxns_reserve_user_qty($entered_item_id, $entered_base_qty);
    }
    // else {
    //   avtxns_reserve_user_qty($entered_item_id, $user_available);
    // }

    //$data['valid_qty'] = !empty($entered_qty_is_valid);
    $data['user_available'] = $user_available + $default_base_qty;
    //$data['total_available'] = $total_available;
  }

  print drupal_json_encode($data);
  drupal_exit();
}

/**
 * Ajax validation if entered qty is less than on-hand qty.
 */
function avtxns_old_qty_check_and_reserve() {
  $data = array(
    //'post' => $_POST,
  );
  extract($_POST);

  $entered_item_id = (int)$entered_item_id;
  //$entered_qty_per_uom = (int)$entered_qty_per_uom;
  $entered_qty_per_uom = 1;
  $transaction_id = (int)$transaction_id;

  if (!empty($entered_item_id)) {
    $default_base_qty = 0;
    if (!empty($transaction_id)) {
      $transaction = avtxns_txn_load($transaction_id);
      if (!empty($transaction->id)) {
        $transaction_items = empty($transaction->items) ? array() : $transaction->items;
        foreach ($transaction_items as $ti) {
          if ($ti->item_id != $entered_item_id) {
            continue;
          }
          $default_qty = empty($ti->qty) ? 0 : $ti->qty;
          $default_qty_per_uom = empty($ti->qty_per_uom) ? 1 : $ti->qty_per_uom;
          $default_base_qty += ($default_qty * $default_qty_per_uom);
        }
      }
    }

    $qty_details = avtxns_item_qty_details($entered_item_id);
    $user_available = (int)$qty_details['user_available'];
    //$total_available = (int)$qty_details['total_available'];
    $entered_base_qty = (is_numeric($entered_qty) ? $entered_qty : 0) * $entered_qty_per_uom;
    if ($entered_base_qty <= $user_available) {
      avtxns_reserve_user_qty($entered_item_id, $entered_base_qty);
    }
    else {
      avtxns_reserve_user_qty($entered_item_id, $user_available);
    }

    //$data['valid_qty'] = !empty($entered_qty_is_valid);
    $data['user_available'] = $user_available + $default_base_qty;
    //$data['total_available'] = $total_available;
  }

  print drupal_json_encode($data);
  drupal_exit();
}

/**
 * Get item on hand qty.
 * @param $item_id
 * @return array|NULL
 */
function avtxns_item_on_hand_qty($item_id) {
  if (empty($item_id)) {
    return NULL;
  }
  // $data = &drupal_static(__FUNCTION__, array());
  // if (!isset($data[$item_id])) {
    $data[$item_id] = db_query("SELECT qty FROM {avtbl_products} WHERE id = :id", array(':id' => $item_id))->fetchField();
  // }
  return $data[$item_id];
}

/**
 * Get total qty reserved for this item.
 * @param $item_id
 * @return array|NULL
 */
function avtxns_user_reserved_qty($item_id) {
  global $user;
  if (empty($item_id) || empty($user->uid)) {
    return NULL;
  }

  // $data = &drupal_static(__FUNCTION__, array());
  // if (!isset($data[$item_id][$user->uid])) {
    $args = array(':item_id' => $item_id, ':expire' => REQUEST_TIME - AVTXN_QTY_RESERVATION_EXPIRE);
    $total_reserved = db_query("SELECT SUM(qty) FROM {avtbl_reserved_qty} WHERE item_id = :item_id AND created >= :expire", $args)->fetchField();
    $args[':uid'] = $user->uid;
    $other_users_reserved = db_query("SELECT SUM(qty) FROM {avtbl_reserved_qty} WHERE item_id = :item_id AND created >= :expire AND uid <> :uid", $args)->fetchField();
    $data[$item_id][$user->uid] = array(
      'total_reserved' => empty($total_reserved) ? 0 : $total_reserved,
      'other_users_reserved' => empty($other_users_reserved) ? 0 : $other_users_reserved,
    );
  // }
  return $data[$item_id][$user->uid];
}

/**
 * Get total qty reserved for this item.
 * @param $item_id
 * @return array|NULL
 */
function avtxns_old_user_reserved_qty($item_id) {
  global $user;
  if (empty($item_id) || empty($user->uid)) {
    return NULL;
  }

  $data = &drupal_static(__FUNCTION__, array());
  $now = time();
  if (!isset($data[$item_id][$user->uid])) {
    $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
    $total_reserved = $other_users_reserved = 0;
    $minutes = 2;
    foreach ($user_reserved_qty as $reserved_item_id => $locked_qty) {
      foreach ($locked_qty as $uid => $dtls) {
        if (empty($uid) || !isset($dtls['qty'])) {
          continue;
        }
        $timestamp = empty($dtls['timestamp']) ? 0 : $dtls['timestamp'];
        $qty = (int) $dtls['qty'];
        if (!empty($qty) && ($timestamp + (60 * $minutes) >= $now)) {
          if ($reserved_item_id == $item_id) {
            $other_users_reserved += ($user->uid && $user->uid == $uid) ? 0 : $dtls['qty'];
            $total_reserved += $dtls['qty'];
          }
        } else {
          unset($user_reserved_qty[$reserved_item_id][$uid]);
        }
      }
      if (empty($user_reserved_qty[$reserved_item_id])) {
        unset($user_reserved_qty[$reserved_item_id]);
      }
    }

    variable_set('avbase_user_reserved_qty', $user_reserved_qty);
    $data[$item_id][$user->uid] = array(
      'total_reserved' => $total_reserved,
      'other_users_reserved' => $other_users_reserved,
    );
  }
  return $data[$item_id][$user->uid];
}

/**
 * Reserve qty for user.
 * @param $item_id
 * @param $qty
 */
function avtxns_old_reserve_user_qty($item_id, $qty) {
  global $user;
  if (!is_numeric($qty) || empty($user->uid)) {
    return;
  }
  $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
  $user_reserved_qty[$item_id][$user->uid]['qty'] = $qty;
  // Reset timestamp of all reservations for this user.
  foreach ($user_reserved_qty as $k => $item_locks) {
    if (!isset($item_locks[$user->uid])) {
      continue;
    }
    $user_reserved_qty[$k][$user->uid]['timestamp'] = time();
  }

  variable_set('avbase_user_reserved_qty', $user_reserved_qty);
}

/**
 * Reserve qty for user.
 * @param $item_id
 * @param $qty
 */
function avtxns_reserve_user_qty($item_id, $qty) {
  global $user;
  if (!is_numeric($qty) || empty($user->uid)) {
    return;
  }

  $now = time();
  // Insert/update reserved quantity.
  db_merge('avtbl_reserved_qty')
    ->key(array('uid' => $user->uid, 'item_id' => $item_id))
    ->fields(array(
      'created' => $now,
      'qty' => $qty,
    ))->execute();

  // Refresh expiration of all reservations for this user.
  db_query("UPDATE {avtbl_reserved_qty} SET created = :time WHERE uid = :uid", array(':time' => $now, ':uid' => $user->uid));
}

/**
 * Remove user's qty reservation for all items.
 */
function avtxns_remove_user_qty_reservation() {
  global $user;
  if (empty($user->uid)) {
    return;
  }

  db_query("DELETE FROM {avtbl_reserved_qty} WHERE uid = :uid", array(':uid' => $user->uid));
}

/**
 * Remove user's qty reservation for all items.
 */
function avtxns_old_remove_user_qty_reservation() {
  global $user;
  if (empty($user->uid)) {
    return;
  }

  $user_reserved_qty = variable_get('avbase_user_reserved_qty', array());
  foreach ($user_reserved_qty as $k => $item_locks) {
    unset($user_reserved_qty[$k][$user->uid]);
  }

  variable_set('avbase_user_reserved_qty', $user_reserved_qty);
}

/**
 * Get item qty details.
 * @param $item_id
 * @return array
 */
function avtxns_item_qty_details($item_id) {
  $on_hand_qty = (int) avtxns_item_on_hand_qty($item_id);
  $reservation = avtxns_user_reserved_qty($item_id);
  return array(
    'on_hand_qty' => $on_hand_qty,
    'user_available' => $on_hand_qty - $reservation['other_users_reserved'],
    'total_available' => $on_hand_qty - $reservation['total_reserved'],
  );
}

/**
 * Return form array containing notes on special discounts.
 * @param $term_days
 * @return array
 */
function avtxns_special_discount_form($term_days) {
  if (empty($term_days)) {
    return array();
  }

  $notes = array();
  for ($x = 1; $x <= 5; $x++) {
    $notes[$term_days * $x] = 0;
  }

  $notes_count = 0;
  $form = array('#theme' => 'avtxns_special_discount_form');
  foreach ($notes as $k => $balance) {
    $form[$k] = array(
      '#type' => 'item',
      '#title' => t('within @to days', array('@to' => $k)),
      '#title_display' => 'inline-before',
      '#markup' => '___________',
    );

    $notes_count++;
    if ($notes_count == count($notes)) {
      $form[$k]['#title'] = t('after @from days', array('@from' => $k - $term_days));
    }
  }
  return $form;
}

/**
 * Return transaction status details.
 * @param $status
 *
 * @return array
 */
function avtxns_txn_status_details($status = NULL) {
  $statuses = array(
    AVTXNS_TXN_STATUS_OPEN => array(
      'status' => AVTXNS_TXN_STATUS_OPEN,
      'title' => t('Open'),
      'badge_class' => 'uk-badge-primary',
      'badge_icon' => 'circle-o',
      'color_class' => '',
    ),
    AVTXNS_TXN_STATUS_LOCKED => array(
      'status' => AVTXNS_TXN_STATUS_LOCKED,
      'title' => t('Locked'),
      'badge_class' => 'uk-badge-primary',
      'badge_icon' => 'lock',
      //'badge_icon' => 'shield',
      //'color_class' => 'uk-alert-success',
    ),
    AVTXNS_TXN_STATUS_CLOSED => array(
      'status' => AVTXNS_TXN_STATUS_CLOSED,
      'title' => t('Closed'),
      'badge_class' => 'uk-badge-success',
      'badge_icon' => 'check-circle',
      'color_class' => 'uk-alert-success',
    ),
    AVTXNS_TXN_STATUS_PENDING => array(
      'status' => AVTXNS_TXN_STATUS_PENDING,
      'title' => t('Pending Approval'),
      'badge_class' => 'uk-badge-warning',
      'badge_icon' => 'minus-circle',
      'color_class' => 'uk-alert-warning',
    ),
    AVTXNS_TXN_STATUS_VOID => array(
      'status' => AVTXNS_TXN_STATUS_VOID,
      'title' => t('Void'),
      'badge_class' => 'uk-badge-void',
      'badge_icon' => 'minus-circle',
      'color_class' => 'uk-row-mute',
    ),
  );

  if (isset($statuses[$status])) {
    return $statuses[$status];
  }
  else {
    return $statuses;
  }
}

/**
 * Change transaction status.
 * @param $id
 * @param $status
 *
 * @return bool
 */
function avtxns_txn_change_status($id, $status) {
  if (empty($id) || !is_numeric($id)) {
    return FALSE;
  }

  $row = avtxns_txn_load($id);
  if (empty($row->id)) {
    drupal_set_message('Transaction not found.', 'error');
    return FALSE;
  }
  $tsd = avtxns_txn_status_details($status);
  if (!isset($tsd['status'])) {
    drupal_set_message('Unknown transaction status.', 'error');
    return FALSE;
  }
  if ($status == $row->status) {
    drupal_set_message(t('Transaction is already @status', array('@status' => $tsd['title'])), 'error');
    return FALSE;
  }

  switch ($row->status) {
    case AVTXNS_TXN_STATUS_CLOSED:
    case AVTXNS_TXN_STATUS_LOCKED:
    case AVTXNS_TXN_STATUS_VOID:
      drupal_set_message('Status of Closed, Locked and Void transactions cannot be changed.', 'error');
      return FALSE;
      break;
    case AVTXNS_TXN_STATUS_OPEN:
      if (!in_array($status, array(AVTXNS_TXN_STATUS_PENDING, AVTXNS_TXN_STATUS_VOID))) {
        drupal_set_message(t('Cannot change transaction to %status', array('%status' => $tsd['title'])), 'error');
        return FALSE;
      }
      break;
    case AVTXNS_TXN_STATUS_PENDING:
      if (!in_array($status, array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_VOID))) {
        drupal_set_message(t('Cannot change transaction to %status', array('%status' => $tsd['title'])), 'error');
        return FALSE;
      }
      break;
  }

  if ($status == AVTXNS_TXN_STATUS_VOID) {
    // Re-open related transactions if possible.
    $item_rows = empty($row->items) ? array() : $row->items;
    $credit_items = empty($row->credit_items) ? array() : $row->credit_items;
    $all_item_rows = $item_rows + $credit_items;
    $related_transaction_ids = array();
    foreach ($all_item_rows as $item_row) {
      $ref_txn_id = empty($item_row->ref_txn_id) ? NULL : $item_row->ref_txn_id;
      if ($ref_txn_id) {
        $related_transaction_ids[$ref_txn_id] = $ref_txn_id;
      }
    }
    // Check if each related transaction IDs can be re-opened.
    //if (!empty($related_transaction_ids)) {
    //  avtxns_txn_reconsider_status($related_transaction_ids);
    //  //$query = "SELECT avtxn_details.ref_txn_id, COUNT(avtxn_details.id) AS active_txn_count
    //  //  FROM {avtbl_transaction_details} avtxn_details
    //  //  INNER JOIN {avtbl_transactions} avtxns ON avtxns.id = avtxn_details.transaction_id
    //  //  WHERE avtxn_details.ref_txn_id IN (:rel_txn_ids) AND avtxn_details.transaction_id <> :txn_id AND avtxns.status <> :txn_status
    //  //  GROUP BY avtxn_details.ref_txn_id";
    //  //$rs = db_query($query, array(':txn_id' => $row->id, ':txn_status' => AVTXNS_TXN_STATUS_VOID, ':rel_txn_ids' => $related_transaction_ids));
    //  //$cannot_open = array();
    //  //foreach ($rs as $rel_row) {
    //  //  $cannot_open[] = $rel_row->ref_txn_id;
    //  //}
    //  //
    //  //$can_be_reopened = array_diff($related_transaction_ids, $cannot_open);
    //}

    // Refresh product quantities.
    avbase_product_qty_update($item_rows, $row, TRUE);
  }

  $now = time();
  if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = :status WHERE id = :txn_id", array(':txn_id' => $row->id, ':status' => $status, ':changed' => $now))) {
    avbase_custom_watchdog("txn__status_change", "ID: {$row->id}", array('#status' => $status, '#row' => $row));

    if (!empty($related_transaction_ids)) {
      avtxns_txn_reconsider_status($related_transaction_ids);
    }

    //if ($status == AVTXNS_TXN_STATUS_VOID) {
    //  // Reopen transactions.
    //  if (!empty($can_be_reopened)) {
    //    if (db_query("UPDATE {avtbl_transactions} SET changed = :changed, status = :status WHERE id IN (:txn_ids)", array(':txn_ids' => $can_be_reopened, ':status' => AVTXNS_TXN_STATUS_OPEN, ':changed' => $now))) {
    //      avbase_custom_watchdog("txn__status_change", "Reopened, IDs: multiple", array('#ids' => $can_be_reopened, '#status' => AVTXNS_TXN_STATUS_OPEN, '#row' => $row));
    //    }
    //  }
    //}

    // Suspend client if necessary.
    if ($status == AVTXNS_TXN_STATUS_OPEN && in_array($row->transaction_type, array(AVTXNS_TXN_TYPE_DELIVERY, AVTXNS_TXN_TYPE_INVOICE))) {
      if ($row->client_status == AVCLIENT_STATUS_ACTIVE) {
        $limit_reached = avbase_client_limit_reached($row->client_id, $row->id);
        if ($limit_reached && avclients_client_change_status($row->client_id, AVCLIENT_STATUS_SUSPENDED)) {
          drupal_set_message(t('Client\'s @credit_limit credit limit has been reached. Transaction can proceed but account has been suspended.', array('@credit_limit' => number_format($row->credit_limit, 2))), 'warning');
        }
      }
    }
    return TRUE;
  }
  return FALSE;
}

/**
 * Change transaction status to open, locked, or closed, depending on related transactions.
 * @param array $ids
 */
function avtxns_txn_reconsider_status($ids = array()) {
  //$transactions = avtxns_txn_load_ids($ids);
  $query = "SELECT avtxns.id, avtxns.status, avtxns.transaction_type, amount_to_credit
    FROM {avtbl_transactions} AS avtxns
    WHERE id IN (:ids)";
  $rs = db_query($query, array(':ids' => $ids));
  $transactions = array();
  foreach ($rs as $row) {
    $transactions[$row->id] = $row;
  }
  if (empty($transactions)) {
    return;
  }

  $credit_txn_types = avtxns_get_creditable_txn_types();


  $update_txns = array();
  foreach ($transactions as $txn) {
    $current_status = $txn->status;
    if (in_array($current_status, array(AVTXNS_TXN_STATUS_VOID, AVTXNS_TXN_STATUS_PENDING))) {
      continue;
    }
    $update_txns[$txn->id] = array(
      'transaction_type' => $txn->transaction_type,
      'reference_count' => 0,
    );

    if (in_array($txn->transaction_type, $credit_txn_types)) {
      $update_txns[$txn->id]['available_credit'] = (float) $txn->amount_to_credit;
    }
    else {
      $ttd = avtxns_txn_types($txn->transaction_type);
      $update_txns[$txn->id]['payable'] = !empty($ttd['payable']);
    }
  }

  if (!empty($update_txns)) {
    $txn_references = avtxns_get_txn_references(array_keys($update_txns));
    foreach ($txn_references as $cu_k => $cu) {
      $update_txns[$cu_k]['reference_count'] = $cu['ref_count'];
      if (isset($update_txns[$cu_k]['available_credit'])) {
        $update_txns[$cu_k]['available_credit'] -= $cu['simple_costs_total'];
      }
    }
  }


  //$query = "SELECT avtxn_dtls.transaction_id, avtxn_dtls.ref_txn_id, avtxn_dtls.cost FROM {avtbl_transaction_details} avtxn_dtls
  //      INNER JOIN {avtbl_transactions} avtxns ON avtxns.id = avtxn_dtls.transaction_id
  //      WHERE avtxn_dtls.ref_txn_id IN (:ids) AND avtxns.status IN (:statuses)";
  //$rs = db_query($query, array(':ids' => array_keys($update_txns), ':statuses' => array(AVTXNS_TXN_STATUS_OPEN, AVTXNS_TXN_STATUS_LOCKED)));
  //foreach ($rs as $row) {
  //  $update_txns[$row->ref_txn_id]['reference_count']++;
  //  if (isset($update_txns[$row->ref_txn_id]['available_credit'])) {
  //    $update_txns[$row->ref_txn_id]['available_credit'] -= $row->cost;
  //  }
  //}

  $change_txn_status = array();
  foreach ($update_txns as $ut_k => $ut) {
    if (isset($ut['reference_count'])) {
      $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_LOCKED;
      if (!empty($ut['payable'])) {
        $refreshed_txn = avtxns_txn_load($ut_k, TRUE);
        $balance = round((float)$refreshed_txn->balance, 2);
        if ($balance <= 0) {
          $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_CLOSED;
        }
        elseif (empty($ut['reference_count'])) {
          $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_OPEN;
        }
      }
      elseif (empty($ut['reference_count'])) {
        $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_OPEN;
      }
      elseif (isset($ut['available_credit'])) {
        $available_credit = round((float)$ut['available_credit'], 2);
        if ($available_credit <= 0) {
          $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_CLOSED;
        }
      }
      elseif (in_array($ut['transaction_type'], array(AVTXNS_TXN_TYPE_SALES_ORDER, AVTXNS_TXN_TYPE_DELIVERY, AVTXNS_TXN_TYPE_PURCHASE_ORDER, AVTXNS_TXN_TYPE_RECEIVE))) {
        $change_txn_status[$ut_k] = AVTXNS_TXN_STATUS_CLOSED;
      }
    }
  }

  $now = time();
  if (!empty($change_txn_status)) {
    $query = "UPDATE {avtbl_transactions} SET changed = :changed, status = CASE id";
    foreach ($change_txn_status as $cts_k => $cts) {
      $query .= " WHEN $cts_k THEN $cts";
    }
    $query .= " END WHERE id IN (" . implode(', ', array_keys($change_txn_status)) . ")";
    if (db_query($query, array(':changed' => $now))) {
      avbase_custom_watchdog("txn__status_change", "IDs: multiple", array('#rows' => $change_txn_status));
    }
  }
}

/**
 * Return transaction type list as options array.
 */
function avtxns_txn_types_options_array($flat = FALSE, $only_these_types = array()) {
  $options = array();
  $types = avtxns_txn_types();
  foreach ($types as $t_k => $t) {
    if (user_access(constant('AVTXNS_PERM_VIEW_' . strtoupper($t_k)))) {
      if (!empty($only_these_types) && !in_array($t_k, $only_these_types)) {
        continue;
      }
      if ($flat) {
        $options[$t_k] = $t['name'];
      }
      else {
        $options[strtoupper($t['transaction'])][$t_k] = $t['name'];
      }
    }
  }
  return $options;
}

/**
 * Return transaction type list as options array.
 */
function avtxns_txn_status_options_array() {
  $options = array();
  $statuses = avtxns_txn_status_details();
  foreach ($statuses as $s_k => $s) {
    $options[$s_k] = $s['title'];
  }
  return $options;
}

/**
 * Advanced transaction load by products.
 * @param array $param_where
 * @param array $param_args
 * @param null $limit
 * @param null $offset
 * @param null $order_by
 * @param $count_query
 *
 * @return array
 */
function avtxns_product_query_load($param_where = array(), $param_args = array(), $limit = NULL, $offset = NULL, $order_by = NULL, $more_select = array(), $more_joins = array(), $group_by = array(), $count_query = FALSE) {
  // Set / load static variable for this query.
  $data = &drupal_static(__FUNCTION__, array());

  // Set and check if data for data_id already exists.
  $func_args = func_get_args();
  foreach ($func_args as $func_k => $func_arg) {
    if (is_array($func_arg)) {
      asort($func_args[$func_k]);
    }
  }
  $data_id = md5(json_encode($func_args));
  if (isset($data[$data_id])) {
    return $data[$data_id];
  }

  $select = array();
  if ($count_query) {
    $select[] = 'COUNT(avproducts.id)';
  }
  else {
    //$select[] = 'avproducts.id';
    //$select[] = 'avproducts.title';
    //$select[] = 'avproducts.qty';
    //$select[] = 'IF(
    //  avtxn.transaction_type = :txn_type_receive,
    //  SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom),
    //  NULL
    //  ) AS qty_purchased';
    //$select[] = 'IF(
    //  avtxn.transaction_type = :txn_type_invoice,
    //  SUM(avtxn_detail.qty * avtxn_detail.qty_per_uom),
    //  NULL
    //  ) AS qty_sold';
    //$select[] = 'IF(
    //  avtxn.transaction_type = :txn_type_receive,
    //  SUM(
    //    avtxn_detail.qty
    //    * avtxn_detail.cost
    //    * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    //    * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    //    * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    //    * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))
    //  ),
    //  NULL
    //  ) AS total_cost';
    //$select[] = 'IF(
    //  avtxn.transaction_type = :txn_type_invoice,
    //  SUM(
    //    avtxn_detail.qty
    //    * avtxn_detail.cost
    //    * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    //    * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    //    * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    //    * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))
    //  ),
    //  NULL
    //  ) AS total_sales';
  }
  $select = array_merge($select, $more_select);

  $joins = array();
  //$joins[] = 'INNER JOIN {avtbl_transaction_details} AS avtxn_detail ON avtxn_detail.item_id = avproducts.id';
  //$joins[] = 'INNER JOIN {avtbl_transactions} AS avtxn ON avtxn.id = avtxn_detail.transaction_id';

  //$joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail_purchase ON avtxn_detail_purchase.item_id = avproducts.id';
  //$joins[] = 'LEFT JOIN {avtbl_transactions} AS avtxn_purchase ON (avtxn_purchase.id = avtxn_detail_purchase.transaction_id AND avtxn_purchase.transaction_type = :avtxn_purchase_ttype)';
  //$joins[] = 'LEFT JOIN {avtbl_clients} AS avclients_vendor ON avclients_vendor.id = avtxn_purchase.client_id';
  //$joins[] = 'LEFT JOIN {avtbl_transaction_details} AS avtxn_detail_sales ON (avtxn_detail_sales.item_id = avproducts.id AND avtxn_detail_sales.ref_txn_type = :avtxn_detail_sales_ttype)';
  //$joins[] = 'LEFT JOIN {avtbl_transactions} AS avtxn_sales ON (avtxn_sales.id = avtxn_detail_sales.transaction_id AND avtxn_sales.transaction_type = :avtxn_sales_ttype)';
  //$joins[] = 'LEFT JOIN {avtbl_clients} AS avclients_customer ON avclients_customer.id = avtxn_sales.client_id';
  //$joins[] = 'LEFT JOIN {avtbl_clients} AS avclients ON (avclients.id = avtxn_purchase.client_id OR avclients.id = avtxn_sales.client_id)';
  //$joins[] = 'LEFT JOIN {avtbl_categories} AS avcat_agent ON (avcat_agent.id = avtxn_purchase.agent_id OR avcat_agent.id = avtxn_sales.agent_id)';

  $joins = array_merge($joins, $more_joins);

  // Args.
  //$param_args[':avtxn_purchase_ttype'] = AVTXNS_TXN_TYPE_RECEIVE;
  //$param_args[':avtxn_detail_sales_ttype'] = AVTXNS_TXN_TYPE_DELIVERY;
  //$param_args[':avtxn_sales_ttype'] = AVTXNS_TXN_TYPE_INVOICE;


  $query = "SELECT " . implode(', ', $select) . " FROM {avtbl_products} AS avproducts " . implode(' ', $joins);

  //$param_where[] = "avtxn_purchase.status IN (" . AVTXNS_TXN_STATUS_LOCKED . ", " . AVTXNS_TXN_STATUS_OPEN . ")";
  //$param_where[] = "avtxn_sales.status IN (" . AVTXNS_TXN_STATUS_LOCKED . ", " . AVTXNS_TXN_STATUS_OPEN . ")";

  //$param_where[] = "avtxn.transaction_type = :txn_type_receive OR (avtxn.transaction_type = :txn_type_invoice AND avtxn_detail.ref_txn_type = :txn_type_delivery)";
  //$param_args[':txn_type_receive'] = AVTXNS_TXN_TYPE_RECEIVE;
  //$param_args[':txn_type_invoice'] = AVTXNS_TXN_TYPE_INVOICE;
  //$param_args[':txn_type_delivery'] = AVTXNS_TXN_TYPE_DELIVERY;
  if (!empty($param_where)) {
    $query .= " WHERE " . implode(' AND ', $param_where);
  }

  if (empty($count_query)) {
    if (!empty($group_by)) {
      $query .= " GROUP BY " . implode(', ', $group_by);
    }
    //$query .= " GROUP BY avtxn.id, avtxn_detail.ref_txn_type";
    $order_by = is_null($order_by) ? array('avproducts.id DESC') : $order_by;
    if (!empty($order_by)) {
      $query .= " ORDER BY " . implode(', ', $order_by);
    }

    if (!is_null($limit)) {
      $query .= " LIMIT $limit";
      if (!is_null($offset)) {
        $query .= " OFFSET $offset";
      }
    }
  }

  if ($count_query) {
    $rows = db_query($query, $param_args)->fetchField();
  }
  else {
    $rs = db_query($query, $param_args);
    $rows = array();
    foreach ($rs as $row) {
      $rows[] = $row;
    }
  }

  $data[$data_id] = $rows;
  return $rows;
}

/**
 * Generate $where and $arg values for date filter.
 * @param $where
 * @param $args
 * @param $date_string
 * @param $date_type
 */
function avtxns_txn_filter_apply_date(&$where, &$args, $date_string, $date_type = 'date_from') {
  $timestamp = strtotime($date_string);
  if (empty($timestamp)) {
    return;
  }

  $placeholder_name = ':' . $date_type . '_stamp';
  if ($date_type == 'date_from') {
    $where[$date_type] = 'avtxn.transaction_date >= ' . $placeholder_name;
  }
  else {
    $timestamp = strtotime('+1 day', $timestamp);
    $where[$date_type] = 'avtxn.transaction_date < ' . $placeholder_name;
  }

  $args[$placeholder_name] = $timestamp;
}

function avtxns_txn_list_filter_date_auto_options_array($date_format = 'F d, Y') {
  $date_auto_options = avtxns_txn_list_filter_date_auto_options($date_format);
  $options = array('custom' => 'Custom');
  foreach ($date_auto_options as $dao_k => $dao) {
    $options[$dao_k] = $dao['title'];
  }
  return $options;
}

/**
 * @param string $date_format
 *
 * @return array
 */
function avtxns_txn_list_filter_date_auto_options($date_format = AVBASE_DATE_FORMAT) {
  $data = &drupal_static(__FUNCTION__, array());
  if (isset($data[$date_format])) {
    return $data[$date_format];
  }

  $now = strtotime(format_date(time(), 'custom', AVBASE_DATE_FORMAT));
  $fiscal_date_timestamp = variable_get('av_fiscal_date', strtotime('January 01'));
  //$fiscal_date_timestamp = strtotime("March 1, 2016");
  //$fiscal_date_timestamp = strtotime("August 29, 2017");
  $fiscal_date_text = format_date($fiscal_date_timestamp, 'custom', 'F d');
  $this_fiscal_date_timestamp = strtotime($fiscal_date_text);
  if ($this_fiscal_date_timestamp > $now) {
    $this_fiscal_date_timestamp = strtotime('-1 year', $this_fiscal_date_timestamp);
  }


  $quarters_start_timestamps = array(
    $this_fiscal_date_timestamp,
    strtotime('+3 months', $this_fiscal_date_timestamp),
    strtotime('+6 months', $this_fiscal_date_timestamp),
    strtotime('+9 months', $this_fiscal_date_timestamp),
    strtotime('+12 months', $this_fiscal_date_timestamp), // do not remove.
  );

  for ($x = 0; !isset($this_quarter_timestamp); $x++) {
    $last_quarter_timestamp = isset($quarters_start_timestamps[$x - 1]) ? $quarters_start_timestamps[$x - 1] : strtotime('-3 months', $this_fiscal_date_timestamp);
    $last_quarter_end_timestamp = strtotime('yesterday', $quarters_start_timestamps[$x]);
    if ($now >= $quarters_start_timestamps[$x] && $now < $quarters_start_timestamps[$x + 1]) {
      $this_quarter_timestamp = $quarters_start_timestamps[$x];
      $this_quarter_end_timestamp = strtotime('yesterday', $quarters_start_timestamps[$x + 1]);
    }
  }


  $this_month_timestamp = strtotime('first day of this month');
  $last_month_timestamp = strtotime('last month', $this_month_timestamp);
  $options = array(
    'today' => array(
      'title' => 'Today',
      'date_from' => format_date($now, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'yesterday' => array(
      'title' => 'Yesterday',
      'date_from' => format_date(strtotime('yesterday'), 'custom', $date_format),
      'date_to' => format_date(strtotime('yesterday'), 'custom', $date_format),
    ),
    'this_week' => array(
      'title' => 'This week',
      'date_from' => format_date(strtotime('last Monday', strtotime('tomorrow')), 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'last_week' => array(
      'title' => 'Last week',
      'date_from' => format_date(strtotime('last Monday', strtotime('last Sunday')), 'custom', $date_format),
      'date_to' => format_date(strtotime('last Sunday'), 'custom', $date_format),
    ),
    'this_month' => array(
      'title' => 'This month',
      'date_from' => format_date($this_month_timestamp, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'this_quarter' => array(
      'title' => 'This fiscal quarter',
      'date_from' => format_date($this_quarter_timestamp, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'this_year' => array(
      'title' => 'This fiscal year',
      'date_from' => format_date($this_fiscal_date_timestamp, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'last_month' => array(
      'title' => 'Last month',
      'date_from' => format_date($last_month_timestamp, 'custom', $date_format),
      'date_to' => format_date(strtotime('last day of this month', $last_month_timestamp), 'custom', $date_format),
    ),
    'last_month_to_date' => array(
      'title' => 'Last month to date',
      'date_from' => format_date($last_month_timestamp, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'last_quarter' => array(
      'title' => 'Last fiscal quarter',
      'date_from' => format_date($last_quarter_timestamp, 'custom', $date_format),
      'date_to' => format_date($last_quarter_end_timestamp, 'custom', $date_format),
    ),
    'last_quarter_to_date' => array(
      'title' => 'Last fiscal quarter to date',
      'date_from' => format_date($last_quarter_timestamp, 'custom', $date_format),
      'date_to' => format_date($now, 'custom', $date_format),
    ),
    'last_year' => array(
      'title' => 'Last fiscal year',
      'date_from' => format_date(strtotime('-1 year', $this_fiscal_date_timestamp), 'custom', $date_format),
      'date_to' => format_date(strtotime('yesterday', $this_fiscal_date_timestamp), 'custom', $date_format),
    ),
  );

  if ($date_format != AVBASE_DATE_FORMAT) {
    unset($options['today']);
    unset($options['yesterday']);
    unset($options['this_week']);
    unset($options['last_week']);
  }

  $data[$date_format] = $options;
  return $data[$date_format];
}

function avtxns_txn_detail_sum_query_discount_applied($increment = 1, $sum = TRUE, $cost = 'avtxn_detail.cost', $true_multiplier = 1, $false_multiplier = -1) {
  // $query = ($increment == 1  ? '' : 'IF(' . $increment . ', 1, -1) * ');
  $query = $increment == 1  ? '' : "IF($increment, $true_multiplier, $false_multiplier) * ";
  $query .= 'avtxn_detail.qty
    * ' . $cost . '
    * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))';
  return $sum ? "SUM($query)" : "($query)";
}

function avtxns_txn_detail_sum_query($increment = 1, $sum = TRUE, $cost = 'avtxn_detail.cost', $true_multiplier = 1, $false_multiplier = -1) {
  // $query = ($increment == 1  ? '' : 'IF(' . $increment . ', 1, -1) * ');
  $query = $increment == 1  ? '' : "IF($increment, $true_multiplier, $false_multiplier) * ";
  $query .= 'avtxn_detail.qty * ' . $cost;
  // return 'SUM(IF(' . $increment . ', 1, -1) * avtxn_detail.qty * avtxn_detail.cost)';
  return $sum ? "SUM($query)" : "($query)";
}

function avtxns_txn_detail_discount_query($sum = FALSE) {
  $amount_query = "(avtxn_detail.qty * avtxn_detail.cost)";
  $query = "$amount_query * (IF(avtxn_detail.discount1, (1 - avtxn_detail.discount1 / 100), 1))
    * (IF(avtxn_detail.discount2, (1 - avtxn_detail.discount2 / 100), 1))
    * (IF(avtxn_detail.discount3, (1 - avtxn_detail.discount3 / 100), 1))
    * (IF(avtxn_detail.discount4, (1 - avtxn_detail.discount4 / 100), 1))";
  $query = "$amount_query - ($query)";
  return $sum ? "SUM($query)" : "($query)";
}

function avtxns_txn_detail_qty_query($increment = 1, $sum = TRUE, $true_multiplier = 1, $false_multiplier = -1) {
  // $query = ($increment == 1  ? '' : 'IF(' . $increment . ', 1, -1) * ');
  $query = $increment == 1  ? '' : "IF($increment, $true_multiplier, $false_multiplier) * ";
  $query .= "avtxn_detail.qty * avtxn_detail.qty_per_uom";
  return $sum ? "SUM($query)" : "($query)";
}

function avtxns_inv_cache_query($type, $sum = FALSE) {
  $qty_change_types = avtxns_get_qty_change_txn_types();
  $all_qty = array();
  foreach ($qty_change_types as $qctype) {
    $all_qty[] = "avinvcache.{$qctype}_qty";
  }
  $end_qty_query = "avinvcache.beg_qty + " . implode(' + ', $all_qty);
  $inventory_debit_query = "(avinvcache.beg_qty + avinvcache.rcv_qty + avinvcache.stin_qty + avinvcache.sr_qty + IF(avinvcache.stk_qty > 0, avinvcache.stk_qty, 0)) * avinvcache.item_cost";
  $inventory_credit_query = "ABS((avinvcache.pr_qty + avinvcache.stou_qty + avinvcache.dr_qty + IF(avinvcache.stk_qty < 0, avinvcache.stk_qty, 0)) * avinvcache.item_cost)";
  $cogafs_debit_query = "avinvcache.beg_amt + avinvcache.rcv_amt + IF(avinvcache.stin_amt > 0, avinvcache.stin_amt, 0)";
  $cogafs_credit_query = "ABS(avinvcache.pr_amt + avinvcache.stou_amt + IF(avinvcache.stin_amt < 0, avinvcache.stin_amt, 0))";
  switch ($type) {
    case 'inventory_debit':
      $query = $inventory_debit_query;
      break;
    case 'inventory_credit':
      $query = $inventory_credit_query;
      break;
    case 'inventory':
      // $query = "($inventory_debit_query) - ($inventory_credit_query)";
      $query = "($end_qty_query) * avinvcache.item_cost";
      break;
    case 'cogafs_debit':
      $query = $cogafs_debit_query;
      break;
    case 'cogafs_credit':
      $query = $cogafs_credit_query;
      break;
    case 'cogafs':
      $query = "avinvcache.beg_amt + avinvcache.rcv_amt + avinvcache.pr_amt + avinvcache.stou_amt + avinvcache.stin_amt";
      break;
    case 'cogs_debit':
      $query = "($cogafs_debit_query) - ($inventory_debit_query)";
      break;
    case 'cogs_credit':
      $query = "($cogafs_credit_query) - ($inventory_credit_query)";
      break;
    case 'cogs':
      $query = "avinvcache.beg_amt + avinvcache.rcv_amt + avinvcache.pr_amt + avinvcache.stou_amt + avinvcache.stin_amt - (($end_qty_query) * avinvcache.item_cost)";
      break;
    case 'end_qty':
      $query = $end_qty_query;
      break;
    default:
      $query = '';
  }

  return $sum ? "SUM($query)" : "($query)";
}

function avtxns_get_item_row_key($item) {
  $reference_type = empty($item->ref_txn_type) ? '' : $item->ref_txn_type;
  return "{$reference_type}_db_item_{$item->transaction_id}_{$item->id}";
}

function avtxns_group_and_sort_item_list($form, $view_mode) {
  if (empty($form['#row_id'])) {
    print drupal_render_children($form);
  }
  else {
    // Group item rows by category title.
    $item_rows_by_category = avtxns_item_rows_by_category($form);


    $item_index = 0;
    foreach ($item_rows_by_category as $category_title => $item_rows) {
      if ($view_mode) {
        print ($item_index == 0 ? '' : '<br />');
      }
      print ($view_mode ? '<div class="uk-badge uk-badge-warning">' . $category_title . '</div>' : '');
      foreach ($item_rows as $item_key => $item_row) {
        $item_rows[$item_key]['#prod_index'] = $item_index++;
      }
      print drupal_render($item_rows);
    }
  }
}
function avtxns_item_rows_by_category($form) {
  $meds = array();
  $meds_cases = array();
  $cases = array();
  $item_rows_by_category = array();
  $no_stock = array();
  foreach (element_children($form) as $item_key) {
    if (is_object($form[$item_key])) {
      $category_title = isset($form[$item_key]->category_title) ? check_plain($form[$item_key]->category_title) : '';
      $uom_title = isset($form[$item_key]->uom_title) ? check_plain($form[$item_key]->uom_title) : NULL;
      $qty = empty($form[$item_key]->qty) ? 0 : $form[$item_key]->qty;
    }
    else {
      $category_title = empty($form[$item_key]['#prod_category']) ? t('No Category') : $form[$item_key]['#prod_category'];
      $uom_title = empty($form[$item_key]['uom_title']['#default_value']) ? '' : $form[$item_key]['uom_title']['#default_value'];
      $qty = empty($form[$item_key]['qty']['#default_value']) ? 0 : $form[$item_key]['qty']['#default_value'];
    }
    $category_title = strtoupper($category_title);


    if (empty($qty)) {
      $no_stock['OUT OF STOCK'][$item_key] = $form[$item_key];
    }
    elseif (in_array($category_title, array('MEDS', 'MEDICINE', 'MEDICINES'))) {
      if (strtoupper($uom_title) == 'CASE') {
        $meds_cases[$category_title . ' - ' . $uom_title][$item_key] = $form[$item_key];
      }
      else {
        $meds[$category_title][$item_key] = $form[$item_key];
      }
    }
    else {
      if (strtoupper($uom_title) == 'CASE') {
        $cases[$uom_title][$item_key] = $form[$item_key];
      }
      else {
        $item_rows_by_category[$category_title][$item_key] = $form[$item_key];
      }
    }
  }
  $item_rows_by_category = $meds  + $meds_cases + $item_rows_by_category + $cases + $no_stock;
  return $item_rows_by_category;
}

function avtxns_get_creditable_txn_types() {
  $credit_txn_types = array();
  $txn_types = avtxns_txn_types();
  foreach ($txn_types as $ttd) {
    if (empty($ttd['credit_txn_types'])) {
      continue;
    }
    foreach ($ttd['credit_txn_types'] as $ctt) {
      $credit_txn_types[$ctt] = $ctt;
    }
  }
  return $credit_txn_types;
}

function avtxns_get_refable_txn_types() {
  $types = array();
  $txn_types = avtxns_txn_types();
  foreach ($txn_types as $ttd) {
    if (empty($ttd['pending_txns_reference_type'])) {
      continue;
    }
    $ptrt = (array) $ttd['pending_txns_reference_type'];
    foreach ($ptrt as $ctt) {
      $types[$ctt] = $ctt;
    }
  }
  return $types;
}

function avtxns_get_payable_txn_types() {
  $types = array();
  $txn_types = avtxns_txn_types();
  foreach ($txn_types as $ttd) {
    if (empty($ttd['payable'])) {
      continue;
    }
    $types[$ttd['type']] = $ttd['type'];
  }
  return $types;
}

function avtxns_get_qty_change_txn_types($change_type = 'all') {
  $types = &drupal_static(__FUNCTION__ . ':' . $change_type, NULL);
  if (isset($types)) {
    return $types;
  }

  $types = array();
  $txn_types = avtxns_txn_types();
  foreach ($txn_types as $ttd) {
    if (empty($ttd['qty_change'])) {
      continue;
    }
    if ($change_type == 'all' || $ttd['qty_change'] == $change_type) {
      $types[] = $ttd['type'];
    }
  }
  return $types;
}

function avtxns_get_txn_references($txn_ids = array()) {
  $query = "SELECT avtxn_dtls.transaction_id, avtxn_dtls.ref_txn_id, avtxn_dtls.cost, avtxns.transaction_type, avtxns.transaction_date
    FROM {avtbl_transaction_details} avtxn_dtls
    INNER JOIN {avtbl_transactions} avtxns ON avtxns.id = avtxn_dtls.transaction_id
    WHERE avtxn_dtls.ref_txn_id IN (:ids) AND avtxns.status <> :void_status";
  $rs = db_query($query, array(':ids' => $txn_ids, ':void_status' => AVTXNS_TXN_STATUS_VOID));
  $references = array();
  foreach ($rs as $row) {
    $references[$row->ref_txn_id]['simple_costs_total'] = isset($references[$row->ref_txn_id]['simple_costs_total']) ? $references[$row->ref_txn_id]['simple_costs_total'] : 0;
    $references[$row->ref_txn_id]['simple_costs_total'] += (float)$row->cost;

    $references[$row->ref_txn_id]['ref_count'] = isset($references[$row->ref_txn_id]['ref_count']) ? $references[$row->ref_txn_id]['ref_count'] : 0;
    $references[$row->ref_txn_id]['ref_count']++;

    $references[$row->ref_txn_id]['refs'][$row->transaction_id] = $row;
  }
  return $references;
}

function avtxns_txn_overdue_query() {
  return '(avtxn.due_date <= UNIX_TIMESTAMP() && avtxn.status != ' . AVTXNS_TXN_STATUS_CLOSED . ' && avtxn.status != ' . AVTXNS_TXN_STATUS_VOID . ')';
}

function avtxns_txn_list_table_add_date_from(&$transaction_list_table, $user_filter) {
  $date_text = empty($user_filter['date_from']) ? '' : check_plain($user_filter['date_from']);
  if (!empty($date_text)) {
    $date_to = empty($user_filter['date_to']) ? '' : check_plain($user_filter['date_to']);
    if (!empty($date_to) && $user_filter['date_from'] != $date_to) {
      $date_text .= ' - ' . $user_filter['date_to'];
    }
    $past_date = empty($date_to) || strtotime($date_to) < strtotime(format_date(time(), 'custom', 'F d, Y'));
    if ($past_date) {
      $date_text = '<span class="uk-color-orange">' . $date_text . '</span>';
    }
    $transaction_list_table['date_from'] = array(
      '#type' => 'item',
      '#markup' => $date_text,
    );
  }
}

function avtxns_txn_list_table_add_product_title(&$transaction_list_table, $user_filter, $transaction_type = NULL) {
  $product_title = empty($user_filter['avprod.title']) ? NULL : $user_filter['avprod.title'];
  if (!empty($product_title)) {
    $transaction_list_table['product_name'] = array(
      '#type' => 'item',
      '#title_display' => 'inline-before',
      '#markup' => '<span class="uk-text-bold">' . check_plain($product_title) . '</span>',
    );
    if ($transaction_type == AVTXNS_TXN_TYPE_SALES_RETURN) {
      $transaction_list_table['product_name']['#markup'] .= '<br />' . avtxns_txn_list_table_pr_link($product_title, $user_filter, 'Purchase returns');
    }
    $transaction_list_table['product_name']['#markup'] .=  '<br />' . avtxns_txn_list_table_phl_link($product_title, $user_filter, 'Stock history');
  }
}

function avtxns_txn_list_table_phl_link($product_title, $user_filter, $link_title = NULL) {
  $link_title = is_null($link_title) ? $product_title : $link_title;
  $link_options = array('html' => TRUE);
  $link_options['query']['custom_reset'] = 1; // Important.
  $link_options['query']['product_title'] = $product_title;
  $link_options['query']['date_auto'] = 'custom';
  $link_options['query']['date_from'] = $user_filter['date_from'];
  //$link_options['query']['date_to'] = $user_filter['date_to'];
  $link_options['attributes']['title'] = 'View stock history';


  return l('<i class="uk-icon-history uk-margin-small-right"></i>' . check_plain($link_title), 'av/reports/stock-history-detailed', $link_options);
}

function avtxns_txn_list_table_pr_link($product_title, $user_filter, $link_title = NULL) {
  $link_title = is_null($link_title) ? $product_title : $link_title;
  $link_options = array('html' => TRUE);
  $link_options['query']['custom_reset'] = 1; // Important.
  $link_options['query']['product_title'] = $product_title;
  $link_options['query']['date_auto'] = 'custom';
  $link_options['query']['date_from'] = $user_filter['date_from'];
  //$link_options['query']['date_to'] = $user_filter['date_to'];
  $link_options['attributes']['title'] = 'View purchase returns';


  return l('<i class="uk-icon-reply uk-margin-small-right"></i>' . check_plain($link_title), 'av/reports/transaction-detail/' . AVTXNS_TXN_TYPE_PURCHASE_RETURN, $link_options);
}

function avtxns_get_types_with_reference_id() {
  return array(
    AVTXNS_TXN_TYPE_PAYMENT,
    AVTXNS_TXN_TYPE_PAY_BILL,
    AVTXNS_TXN_TYPE_VENDOR_CREDIT,
    AVTXNS_TXN_TYPE_CREDIT_MEMO,
    AVTXNS_TXN_TYPE_OTHER_BILLS,
    AVTXNS_TXN_TYPE_CUSTOMER_CHARGES,
    AVTXNS_TXN_TYPE_RECEIVE,
    AVTXNS_TXN_TYPE_PURCHASE_RETURN,
    AVTXNS_TXN_TYPE_SALES_RETURN,
    AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN,
    AVTXNS_TXN_TYPE_JOURNAL,
  );
}

function avtxns_get_accounts_txn_types() {
  $types = array();
  $txn_types = avtxns_txn_types();
  foreach ($txn_types as $ttd) {
    if ($ttd['transaction'] != 'accounts') {
      continue;
    }
    $types[$ttd['type']] = $ttd['type'];
  }
  return $types;
}

function avtxns_get_types_with_account_id($account_id) {
  static $data;
  if (isset($data[$account_id])) {
    return $data[$account_id];
  }

  $rows = array();
  $types = avtxns_txn_types();
  foreach ($types as $type_k => $type) {
    if (!empty($type['account_ids'])) {
      $refs = $type['account_ids'];
      foreach ($refs as $ref_txn_type => $accounts) {
        foreach ($accounts as $acc_k => $acc_v) {
          if ($acc_k == $account_id) {
            $rows[$type_k][$ref_txn_type] = $acc_v;
          }
        }
      }
    }
    elseif (!empty($type['default_account_id'])) {
      $ref_txn_type = 'default_account_id';
      $accounts = $type['default_account_id'];
      foreach ($accounts as $acc_k => $acc_v) {
        if ($acc_k == $account_id) {
          $rows[$type_k][$ref_txn_type] = $acc_v;
        }
      }
    }
  }
  $data[$account_id] = $rows;
  return $rows;
}

function avtxns_get_txn_account_types() {//meeee
  $ttypes = array();
  $transaction_types = avtxns_txn_types();
  foreach ($transaction_types as $t_k => $ttd) {
    if ($ttd['transaction'] == 'accounts') {
      $ttypes[] = $t_k;
    }
  }
  return $ttypes;
}

/**
 * Scan all transactions related to specified account id.
 * @param $current_account_id
 * @param $account_type
 * @param $date_start
 * @param $date_end
 */
function avtxns_read_account_transactions($current_account_id, $account_type, $date_start, $date_end) {
  // if (!in_array($current_account_id, array(AVTXN_AC_SPECIFIC_INVENTORY_ID, AVTXN_AC_SPECIFIC_COGS_ID))) {
  //   return;
  // }

  $debit_amount = $credit_amount = 0;
  if (in_array($current_account_id, array(AVTXN_AC_SPECIFIC_INVENTORY_ID, AVTXN_AC_SPECIFIC_COGS_ID))) {
    $fc_increment_types = array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN);
    $fc_decrement_types = array(AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT);
    $fixed_cost_types = array_merge($fc_increment_types, $fc_decrement_types);
    $qty_change_types = avtxns_get_qty_change_txn_types();
    // $select = array('item_cost', "SUM(beg_amt) beg_amt");
    $select = array();
    $select[] = avtxns_inv_cache_query('inventory_debit', TRUE) . " inventory_debit";
    $select[] = avtxns_inv_cache_query('inventory_credit', TRUE) . " inventory_credit";
    $select[] = avtxns_inv_cache_query('cogs_debit', TRUE) . " cogs_debit";
    $select[] = avtxns_inv_cache_query('cogs_credit', TRUE) . " cogs_credit";
    $query = "SELECT " . implode(', ', $select) . "
      FROM {avtbl_inventory_cache} avinvcache
      WHERE before_this_day = $date_end";
    $data = db_query($query)->fetchObject();
    if (empty($data)) {
      return;
    }
    if ($current_account_id == AVTXN_AC_SPECIFIC_INVENTORY_ID) {
      $debit_amount = empty($data->inventory_debit) ? 0 : $data->inventory_debit;
      $credit_amount = empty($data->inventory_credit) ? 0 : $data->inventory_credit;
    }
    else {
      $debit_amount = empty($data->cogs_debit) ? 0 : $data->cogs_debit;
      $credit_amount = empty($data->cogs_credit) ? 0 : $data->cogs_credit;
    }
  }
  else {
    $actd = avtxns_account_chart_types($account_type);
    $account_balance_type = $actd['balance_type'];
    $types_with_account_id = avtxns_get_types_with_account_id($current_account_id);
    $cases = array();
    $where = array();
    foreach ($types_with_account_id AS $_txn_type => $_refs) {
      foreach ($_refs AS $_ref_ttype => $_v) {
        $case_conditions = array();
        $case_conditions[] = "avtxn.transaction_type = '$_txn_type'";
        if (in_array($_ref_ttype, array('default', 'default_account_id'))) {
          $case_conditions[] = "avtxn_detail.ref_txn_type IS NULL";
        }
        else {
          $case_conditions[] = "avtxn_detail.ref_txn_type = '$_ref_ttype'";
        }

        $multiplier = $_v == $account_balance_type ? 1 : -1;
        if (in_array($current_account_id, array(AVTXN_AC_SPECIFIC_AR_ID, AVTXN_AC_SPECIFIC_AP_ID, AVTXN_AC_SPECIFIC_SALES_RETURN_ID, AVTXN_AC_SPECIFIC_COGS_ID))) {
          $amount_query = avtxns_txn_detail_sum_query_discount_applied(1, FALSE);
        }
        elseif ($current_account_id == AVTXN_AC_SPECIFIC_SALES_DISCOUNT_ID) {
          $amount_query = avtxns_txn_detail_discount_query(FALSE);
        }
        else {
          $amount_query = avtxns_txn_detail_sum_query(1, FALSE);
        }

        // $cases[$multiplier][] = "WHEN " . implode(' AND ', $case_conditions) . " THEN ($multiplier * (" . $amount_query . "))";
        $cases[$multiplier][] = "WHEN " . implode(' AND ', $case_conditions) . " THEN (" . $amount_query . ")";
      }
    }

    // if (!in_array($current_account_id, array(AVTXN_AC_SPECIFIC_AR_ID, AVTXN_AC_SPECIFIC_AP_ID))) {
    $account_txn_types = avtxns_get_txn_account_types();
    foreach ($account_txn_types as $k) {
      if (isset($types_with_account_id[$k])) {
        continue;
      }
      $this_account_txn_type = avtxns_txn_types($k);
      $types_with_account_id[$k] = NULL;
      $_case = "WHEN (avtxn.transaction_type = '$k' AND avtxn_detail.account_id = $current_account_id) THEN ";
      if (!empty($this_account_txn_type['default_account_id'])) {
        $balance_type = array_shift($this_account_txn_type['default_account_id']);
        $multiplier = $balance_type == $account_balance_type ? -1 : 1;
        // $_case .= "(avtxn_detail.cost * $multiplier)" ;
        $_case .= "(avtxn_detail.cost)";
        $cases[$multiplier][] = $_case;
      }
      else {
        // $multiplier = $account_balance_type == 'credit' ? -1 : 1;
        // $multiplier = $account_balance_type == 'credit' ? 1 : -1;
        // $_case .= "((avtxn_detail.debit - avtxn_detail.credit) * $multiplier)";
        // $_case .= "(avtxn_detail.debit - avtxn_detail.credit)";
        $cases[1][] = $_case . "(avtxn_detail.debit)";
        $cases[-1][] = $_case . "(avtxn_detail.credit)";
      }
      // $cases[$multiplier][] = $_case;
    }
    // }

    $query = "SELECT SUM(CASE " . implode(' ', $cases[1]) . " END) AS debit, SUM(CASE " . implode(' ', $cases[-1]) . " END) AS credit";
    $query .= " FROM avtbl_transactions avtxn
      INNER JOIN avtbl_transaction_details avtxn_detail ON avtxn_detail.transaction_id = avtxn.id
      WHERE avtxn.transaction_type IN ('" . implode("','",array_keys($types_with_account_id)) . "') AND avtxn.transaction_date >= $date_start AND avtxn.transaction_date < $date_end
      AND avtxn.status NOT IN (" . AVTXNS_TXN_STATUS_PENDING . ',' . AVTXNS_TXN_STATUS_VOID . ")";
    // 1517414400
    if (!empty($where)) {
      $query .= " AND " . implode(' AND ', $where);
    }

    $row = db_query($query)->fetchObject();
    $debit_amount = empty($row->debit) ? 0 : $row->debit;
    $credit_amount = empty($row->credit) ? 0 : $row->credit;
  }

  if (!empty($debit_amount) || !empty($credit_amount)) {
    $history_row = array();
    $history_row['account_id'] = $current_account_id;
    $history_row['before_this_day'] = $date_end;
    $history_row['amount'] = 0;
    $history_row['debit'] = $debit_amount;
    $history_row['credit'] = $credit_amount;
    db_insert('avtbl_fs')->fields($history_row)->execute();
  }
}

/**
 * Scan all transactions related to specified product id.
 * @param $selected_product
 * @param $date_start
 * @param $date_end
 */
function avtxns_read_product_transactions($selected_product, $date_start, $date_end) {
  // Make sure we only start on products initial qty date.
  if ($selected_product->initial_qty_date > $date_end) {
    return;
  }

  $do_insert = FALSE;
  $history_row = array();
  $history_row['product_id'] = $selected_product->id;
  $history_row['before_this_day'] = $date_end;
  // $history_row['item_cost'] = $item_cost;
  $history_row['qty_out'] = 0; // remove this.
  $history_row['qty_in'] = 0; // remove this.
  $history_row['qty_in_amount'] = 0; // remove this.

  $fc_increment_types = array(AVTXNS_TXN_TYPE_RECEIVE, AVTXNS_TXN_TYPE_STOCK_TRANSFER_IN);
  $fc_decrement_types = array(AVTXNS_TXN_TYPE_PURCHASE_RETURN, AVTXNS_TXN_TYPE_STOCK_TRANSFER_OUT);
  $fixed_cost_types = array_merge($fc_increment_types, $fc_decrement_types);
  $qty_change_types = avtxns_get_qty_change_txn_types();

  // Get qty for each transaction types.
  $type_queries = array();
  foreach ($qty_change_types as $qctype) {
    if ($qctype == AVTXNS_TXN_TYPE_STOCK_ADJUSTMENT) {
      $type_multiplier = 1;
    }
    else {
      $ttd = avtxns_txn_types($qctype);
      $inventory_account_type = isset($ttd['account_ids']['default'][AVTXN_AC_SPECIFIC_INVENTORY_ID]) ? $ttd['account_ids']['default'][AVTXN_AC_SPECIFIC_INVENTORY_ID] : '';
      if (empty($inventory_account_type)) {
        continue;
      }
      $type_multiplier = $inventory_account_type == 'credit' ? -1 : 1;
    }

    if (in_array($qctype, $fixed_cost_types)) {
      $type_queries[$qctype . '_amt'] = avtxns_txn_detail_sum_query_discount_applied("avtxn.transaction_type = '$qctype'", TRUE, 'avtxn_detail.cost', 1, 0) . " * $type_multiplier {$qctype}_amt";
    }
    $type_queries[$qctype . '_qty'] = avtxns_txn_detail_qty_query("avtxn.transaction_type = '$qctype'", TRUE, 1, 0) . " * $type_multiplier {$qctype}_qty";
  }

  $query = "SELECT " . implode($type_queries, ', ') . "
    FROM {avtbl_transaction_details} avtxn_detail
    INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
    WHERE avtxn_detail.item_id = :item_id
    AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:this_types)
    AND avtxn.transaction_date >= :date_from
    AND avtxn.transaction_date < :date_to";
  if (!empty($order_by)) {
    $query .= " ORDER BY " . implode(', ', $order_by);
  }
  $row = db_query($query, array(
    ':item_id' => $selected_product->id,
    ':void_status' => AVTXNS_TXN_STATUS_VOID,
    ':this_types' => $qty_change_types,
    ':date_from' => $date_start,
    ':date_to' => $date_end,
  ))->fetchObject();

  // Assign retrieved values into $history_row array.
  foreach ($qty_change_types as $qctype) {
    $_varname = $qctype . '_amt';
    if (isset($row->$_varname)) {
      $history_row[$_varname] = $row->$_varname;
      $do_insert = TRUE;
    }
    $_varname = $qctype . '_qty';
    if (isset($row->$_varname)) {
      $history_row[$_varname] = $row->$_varname;
      $do_insert = TRUE;
    }
  }

  // If product's initial quantity date is within start and end date, use it as beginning quantity.
  if ($selected_product->initial_qty_date >= $date_start && $selected_product->initial_qty_date < $date_end) {
    $beg_qty = $selected_product->initial_qty;
    $do_insert = TRUE;
  }
  elseif ($do_insert) {
    // Get from last data in inventory cache.
    $beg_qty = 0;
    $query = "SELECT * FROM {avtbl_inventory_cache} WHERE product_id = :product_id ORDER BY before_this_day DESC LIMIT 1";
    $last_cache_row = db_query($query, array(':product_id' => $selected_product->id))->fetchObject();
    if (isset($last_cache_row->beg_qty)) {
      $beg_qty += $last_cache_row->beg_qty;
      foreach ($qty_change_types as $qctype) {
        $_varname = $qctype . '_qty';
        if (!isset($last_cache_row->$_varname)) {
          continue;
        }
        $beg_qty += $last_cache_row->$_varname;
      }
    }
  }


  if ($do_insert) {
    // Last item cost used based on already saved data inside cache table.
    // Use beginning cost if no cached data yet.
    if (empty($last_cache_row->item_cost)) {
      $item_cost = $selected_product->item_cost;
      $init_date = strtotime('today', $selected_product->initial_qty_date);
    }
    else {
      $item_cost = $last_cache_row->item_cost;
      $init_date = $date_start;
    }
    $beg_amt = $beg_qty * $item_cost;

    // Get most recent product cost based on selected end date.
    // Use last cached item cost used if no incoming relevant transactions yet.
    $item_cost_query = "SELECT (avtxn_detail.cost / avtxn_detail.qty_per_uom) item_cost
    FROM {avtbl_transaction_details} avtxn_detail
    INNER JOIN {avtbl_transactions} avtxn ON avtxn.id = avtxn_detail.transaction_id
    WHERE avtxn_detail.item_id = :item_id
    AND avtxn.status <> :void_status AND avtxn.transaction_type IN (:fixed_cost_types)
    AND avtxn.transaction_date >= :init_qty_date
    AND avtxn.transaction_date < :date_to
    ORDER BY avtxn.transaction_date DESC, avtxn.id DESC LIMIT 1";
    $item_cost_args = array(
      ':item_id' => $selected_product->id,
      ':void_status' => AVTXNS_TXN_STATUS_VOID,
      ':fixed_cost_types' => $fixed_cost_types,
      ':init_qty_date' => $init_date,
      ':date_to' => $date_end,
    );
    $found_item_cost = db_query($item_cost_query, $item_cost_args)->fetchField();
    $item_cost = empty($found_item_cost) ? $item_cost : $found_item_cost;

    // Finally, insert history row into cache table.
    $history_row['beg_qty'] = $beg_qty;
    $history_row['beg_amt'] = $beg_amt;
    $history_row['item_cost'] = $item_cost;
    db_insert('avtbl_inventory_cache')->fields($history_row)->execute();
  }
}
