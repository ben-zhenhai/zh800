package tw.com.zhenhai.db

import tw.com.zhenhai.model._

import com.mongodb.casbah.Imports._

import java.text.SimpleDateFormat
import java.util.Date

class MongoProcessor(mongoClient: MongoClient) {

  val zhenhaiDB = mongoClient("zhenhai")
  val dailyDB = mongoClient("zhenhaiDaily")
  val dateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm")

  def update(tableName: String, query: MongoDBObject, record: Record) {
    val operation = $inc("bad_qty" -> record.badQty, "count_qty" -> record.countQty)
    zhenhaiDB(tableName).ensureIndex(query.mapValues(x => 1))
    zhenhaiDB(tableName).update(query, operation, upsert = true)
  }

  def addMachineAlert(record: Record) {

    val query = MongoDBObject(
      "timestamp" -> dateFormatter.format(new Date(record.embDate * 1000)),
      "mach_id"   -> record.machID
    )

    zhenhaiDB("alert").update(query, query, upsert = true);
  }

  def addRecord(record: Record, isImportFromDaily: Boolean = false) {
    val tenMinute = dateFormatter.format(record.embDate * 1000).substring(0, 15) + "0"

    update(
      tableName = "product", 
      query = MongoDBObject("product" -> record.product), 
      record = record
    )

    update(
      tableName = s"product-${record.product}", 
      query = MongoDBObject(
        "timestamp" -> record.insertDate, 
        "shiftDate" -> record.shiftDate, 
        "mach_id" -> record.machID
      ), 
      record = record
    )

    update(
      tableName = record.insertDate, 
      query = MongoDBObject(
        "timestamp" -> tenMinute, 
        "product" -> record.product, 
        "mach_id" -> record.machID, 
        "defact_id" -> record.defactID
      ), 
      record = record
    )

    update(
      tableName = s"shift-${record.shiftDate}", 
      query = MongoDBObject(
        "timestamp" -> tenMinute, 
        "product" -> record.product, 
        "mach_id" -> record.machID, 
        "defact_id" -> record.defactID
      ), 
      record = record
    )

    update(
      tableName = "dailyDefact", 
      query = MongoDBObject(
        "timestamp" -> record.insertDate, 
        "shiftDate" -> record.shiftDate, 
        "mach_id" -> record.machID, 
        "defact_id" -> record.defactID
      ), 
      record = record
    )

    update(
      tableName = "daily", 
      query = MongoDBObject(
        "timestamp" -> record.insertDate, 
        "shiftDate" -> record.shiftDate, 
        "mach_id" -> record.machID
      ), 
      record = record
    )

    update(
      tableName = "reasonByMachine", 
      query = MongoDBObject(
        "mach_id" -> record.machID,
        "mach_model" -> MachineInfo.getModel(record.machID),
        "mach_type" -> MachineInfo.getMachineType(record.machID)
      ), 
      record = record
    )

    zhenhaiDB("data").insert(record.toMongoObject)

    if (!isImportFromDaily) {
      dailyDB(record.insertDate).insert(record.toMongoObject)
    }
  }
}

