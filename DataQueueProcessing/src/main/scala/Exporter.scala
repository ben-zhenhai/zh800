import java.sql.Connection
import com.mongodb.casbah.Imports._

class Exporter(connection: Connection) {

  val mongoClient = MongoClient("localhost", 27017)
  val dailyDB = mongoClient("daily")
  val monthlyDB = mongoClient("monthly")

  dailyDB.dropDatabase()
  monthlyDB.dropDatabase()

  def exportProduct() {
    val statement = connection.prepareStatement("SELECT product, count_qty, bad_qty FROM product")
    val resultSet = statement.executeQuery()
    val collection = dailyDB("product")

    while (resultSet.next()) {
      val data = MongoDBObject(
        "product" -> resultSet.getString(1), 
        "count_qty" -> resultSet.getLong(2), 
        "bad_qty" -> resultSet.getLong(3)
      )
      collection.insert(data)
    }

    resultSet.close()
    statement.close()
  }

  def exportProductDaily() {
    val statement = connection.prepareStatement("SELECT product, mach_id, timestamp, count_qty, bad_qty FROM product_daily")
    val resultSet = statement.executeQuery()

    while (resultSet.next()) {
      val product = resultSet.getString(1)
      val machineID = resultSet.getString(2);
      val timestamp = resultSet.getString(3);
      val countQty = resultSet.getLong(4);
      val badQty = resultSet.getLong(5);
      val collection = dailyDB(s"product-$product")

      val data = MongoDBObject(
        "mach_id" -> machineID,
        "timestamp" -> timestamp,
        "count_qty" -> countQty,
        "bad_qty" -> badQty
      )
      println("insert " + data)
      collection.insert(data)
    }

    resultSet.close()
    statement.close()
  }

  def exportInterval() {
    val statement = connection.prepareStatement("SELECT timestamp, product, mach_id, defact_id, count_qty, bad_qty FROM interval")
    val resultSet = statement.executeQuery()

    while (resultSet.next()) {
      val timestamp = resultSet.getString(1);
      val product = resultSet.getString(2);
      val machineID = resultSet.getString(3);
      val defactID = resultSet.getLong(4);
      val countQty = resultSet.getLong(5);
      val badQty = resultSet.getLong(6);
      val dateString = timestamp.substring(0, 10);
      val collection = dailyDB(dateString)

      val data = MongoDBObject(
        "timestamp" -> timestamp,
        "product" -> product,
        "mach_id" -> machineID,
        "defact_id" -> defactID,
        "count_qty" -> countQty,
        "bad_qty" -> badQty
      )
      println("insert " + data)
      collection.insert(data)
    }

    resultSet.close()
    statement.close()
  }

  def exportDaily() {
    val statement = connection.prepareStatement("SELECT mach_id, timestamp, count_qty, bad_qty FROM daily")
    val resultSet = statement.executeQuery()

    while (resultSet.next()) {
      val machineID = resultSet.getString(1);
      val timestamp = resultSet.getString(2);
      val countQty = resultSet.getLong(3);
      val badQty = resultSet.getLong(4);
      val collection = dailyDB("daily")

      val data = MongoDBObject(
        "mach_id" -> machineID,
        "timestamp" -> timestamp,
        "count_qty" -> countQty,
        "bad_qty" -> badQty
      )
      println("insert " + data)
      collection.insert(data)
    }

    resultSet.close()
    statement.close()
  }

  def exportMonthly() {
    val statement = connection.prepareStatement("SELECT timestamp, mach_id, defact_id, count_qty, bad_qty FROM monthly")
    val resultSet = statement.executeQuery()
    val collection = monthlyDB("monthly")

    while (resultSet.next()) {
      val timestamp = resultSet.getString(1);
      val machineID = resultSet.getString(2);
      val defactID = resultSet.getLong(3);
      val countQty = resultSet.getLong(4);
      val badQty = resultSet.getLong(5);
      val data = MongoDBObject(
        "timestamp" -> timestamp,
        "mach_id" -> machineID,
        "defact_id" -> defactID,
        "count_qty" -> countQty,
        "bad_qty" -> badQty
      )
      println("insert " + data)
      collection.insert(data)
    }

    resultSet.close()
    statement.close()
  }


  def export() {
    exportProduct()
    exportProductDaily()
    exportInterval()
    exportDaily()
    exportMonthly()
  }


}

