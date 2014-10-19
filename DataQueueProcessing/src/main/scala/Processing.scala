import com.mongodb.casbah.Imports._
import java.text.SimpleDateFormat

class MongoProcessor {

  val mongoClient = MongoClient("localhost")
  val mongoDaily = mongoClient("daily")
  val mongoMonthly = mongoClient("monthly")
  val dateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm")

  def update(mongoDB: MongoDB, tableName: String, query: MongoDBObject, record: Record) {
    val operation = $inc("bad_qty" -> record.badQty, "count_qty" -> record.countQty)
    mongoDB(tableName).ensureIndex(query.mapValues(x => 1))
    mongoDB(tableName).update(query, operation, upsert = true)
  }

  def addRecord(record: Record) {
    val tenMinute = dateFormatter.format(record.embDate * 1000).substring(0, 15) + "0"

    update(
      mongoDB = mongoDaily, 
      tableName = "product", 
      query = MongoDBObject("product" -> record.product), 
      record = record
    )

    update(
      mongoDB = mongoDaily, 
      tableName = s"product-${record.product}", 
      query = MongoDBObject("timestamp" -> record.insertDate, "mach_id" -> record.machID), 
      record = record
    )

    update(
      mongoDB = mongoDaily, 
      tableName = record.insertDate, 
      query = MongoDBObject("timestamp" -> tenMinute, "product" -> record.product, "mach_id" -> record.machID, 
                            "defact_id" -> record.defactID), 
      record = record
    )

    update(
      mongoDB = mongoMonthly, 
      tableName = "monthly", 
      query = MongoDBObject("timestamp" -> record.insertDate.substring(0, 7), "mach_id" -> record.machID, 
                            "defact_id" -> record.defactID), 
      record = record
    )

    update(
      mongoDB = mongoDaily, 
      tableName = "daily", 
      query = MongoDBObject("timestamp" -> record.insertDate, "mach_id" -> record.machID), 
      record = record
    )

    mongoMonthly("raw").insert(record.toMongoObject)
  }

}


