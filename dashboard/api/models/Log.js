/**
* Log.js
*
* @description :: TODO: You might write a short summary of how this model works and what it represents here.
* @docs        :: http://sailsjs.org/#!documentation/models
*/

module.exports = {

  schema: true,
  attributes: {
    order_type: { type: "string", required: true, unique: true },
    lot_no: { "type": "string", required: true, unique: true },
    work_qty:  { "type": "integer", required: true },
    count_qty: { "type": "integer", required: true },
    emb_date:  { "type": "date", required: true },
    bad_qty:   { "type": "integer", required: true },
    mach_ip:   { "type": "string", required: true },
    defact_id: { "type": "integer", required: true },
    mach_id:   { "type": "string", required: true },
    work_id:   { "type": "integer", required: true },
    cx:        { "type": "string", required: true },
    dx:        { "type": "string", required: true },
    lc:        { "type": "string", required: true },
    mach_status: { "type": "string", required: true }
  }

};

