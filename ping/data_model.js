var mongoose = require('mongoose')
var Schema = mongoose.Schema

var DataSchema = new Schema(
    {
        order_type : String,
        lot_no : String,
        work_qty : String,
        count_qty : String,
        emb_date : String,
        bad_qty : String,
        mach_ip : String,
        defact_id : String,
        mach_id : String,
        work_id : String,
        CX : String, 
        DX : String,
        LC : String,
        mach_status : String
    },
    {
        versionKey: false
    }
)

module.exports = {
    getModel: function getModel(connection) {
        return connection.model('Data', DataSchema, 'data')
    }
}

/*
 * 單別
 * 制令編號
 * 投產量
 * 計數量
 * 時間
 * 不良數量
 * 機台
 * 不良原因號碼
 * 機台號碼
 * 員工工號
 * CX
 * DX
 * LC
 * 機台狀態
 */
