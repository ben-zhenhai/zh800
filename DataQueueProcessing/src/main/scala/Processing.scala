import scala.io.Source
import java.io.File
import java.sql.Connection
import java.sql.DriverManager
import java.util.Calendar
import java.util.Date
import java.text.SimpleDateFormat


class AddToPostgres(connection: Connection) {

  def processDaily(record: Record) {

    val dateFormatter = new SimpleDateFormat("yyyy-MM-dd")
    val dateTimestamp = dateFormatter.format(new Date(record.embDate * 1000))

    def createNewRecord() {
      val statement = connection.prepareStatement("INSERT INTO daily (timestamp, mach_id, count_qty, bad_qty) VALUES(?, ?, ?, ?)");
      statement.setString(1, dateTimestamp);
      statement.setString(2, record.machID);
      statement.setLong(3, record.countQty);
      statement.setLong(4, record.badQty);
      statement.executeUpdate();
      statement.close();
    }

    def updateRecord(countQty: Long, badQty: Long) {
      val statement = connection.prepareStatement("UPDATE daily SET count_qty=?, bad_qty =? WHERE timestamp=? AND mach_id=?");
      statement.setLong(1, countQty);
      statement.setLong(2, badQty);
      statement.setString(3, dateTimestamp);
      statement.setString(4, record.machID);
      statement.executeUpdate();
      statement.close();
    }

    def getData(record: Record): Option[(Long, Long)] = {
      var data: Option[(Long, Long)] = None
      val statement = connection.prepareStatement("SELECT count_qty, bad_qty FROM daily WHERE timestamp=? AND mach_id=?");
      statement.setString(1, dateTimestamp)
      statement.setString(2, record.machID);
      val result = statement.executeQuery();
      if (result.next) {
        data = Some((result.getLong(1), result.getLong(2)))
      }
      statement.close();
      result.close();
      data
    }

    getData(record) match {
      case None => createNewRecord()
      case Some((countQty, badQty)) => updateRecord(countQty + record.countQty, badQty + record.badQty)
    }

  }

  def processInterval(record: Record) {

    val dateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm")
    val dateTimestamp = dateFormatter.format(new Date(record.embDate * 1000)).substring(0, 15) + "0"

    def createNewRecord() {
      val statement = connection.prepareStatement("INSERT INTO interval (timestamp, mach_id, defact_id, product, count_qty, bad_qty) VALUES(?, ?, ?, ?, ?, ?)");
      statement.setString(1, dateTimestamp);
      statement.setString(2, record.machID);
      statement.setLong(3, record.defactID);
      statement.setString(4, record.product);
      statement.setLong(5, record.countQty);
      statement.setLong(6, record.badQty);
      statement.executeUpdate();
      statement.close();
    }

    def updateRecord(countQty: Long, badQty: Long) {
      val statement = connection.prepareStatement("UPDATE interval SET count_qty=?, bad_qty =? WHERE timestamp=? AND mach_id=? AND defact_id=? AND product=?");
      statement.setLong(1, countQty);
      statement.setLong(2, badQty);
      statement.setString(3, dateTimestamp);
      statement.setString(4, record.machID);
      statement.setLong(5, record.defactID);
      statement.setString(6, record.product);
      statement.executeUpdate();
      statement.close();
    }

    def getData(record: Record): Option[(Long, Long)] = {
      var data: Option[(Long, Long)] = None
      val statement = connection.prepareStatement("SELECT count_qty, bad_qty FROM interval WHERE timestamp=? AND mach_id=? AND defact_id=? AND product=?");
      statement.setString(1, dateTimestamp)
      statement.setString(2, record.machID);
      statement.setLong(3, record.defactID);
      statement.setString(4, record.product);
 
      val result = statement.executeQuery();
      if (result.next) {
        data = Some((result.getLong(1), result.getLong(2)))
      }
      statement.close();
      result.close();
      data
    }

    getData(record) match {
      case None => createNewRecord()
      case Some((countQty, badQty)) => updateRecord(countQty + record.countQty, badQty + record.badQty)
    }

  }

  def processProductDaily(record: Record) {

    val dateFormatter = new SimpleDateFormat("yyyy-MM-dd")
    val dateTimestamp = dateFormatter.format(new Date(record.embDate * 1000))

    def createNewRecord() {
      val statement = connection.prepareStatement("INSERT INTO product_daily (product, timestamp, mach_id, count_qty, bad_qty) VALUES(?, ?, ?, ?, ?)");
      statement.setString(1, record.product)
      statement.setString(2, dateTimestamp);
      statement.setString(3, record.machID);
      statement.setLong(4, record.countQty);
      statement.setLong(5, record.badQty);
      statement.executeUpdate();
      statement.close();
    }

    def updateRecord(countQty: Long, badQty: Long) {
      val statement = connection.prepareStatement("UPDATE product_daily SET count_qty=?, bad_qty =? WHERE product=? AND timestamp=? AND mach_id=?");
      statement.setLong(1, countQty);
      statement.setLong(2, badQty);
      statement.setString(3, record.product);
      statement.setString(4, dateTimestamp);
      statement.setString(5, record.machID);
      statement.executeUpdate();
      statement.close();
    }

    def getData(record: Record): Option[(Long, Long)] = {
      var data: Option[(Long, Long)] = None
      val statement = connection.prepareStatement("SELECT count_qty, bad_qty FROM product_daily WHERE timestamp=? AND mach_id=? AND product=?");
      statement.setString(1, dateTimestamp)
      statement.setString(2, record.machID);
      statement.setString(3, record.product);
 
      val result = statement.executeQuery();
      if (result.next) {
        data = Some((result.getLong(1), result.getLong(2)))
      }
      statement.close();
      result.close();
      data
    }

    getData(record) match {
      case None => createNewRecord()
      case Some((countQty, badQty)) => updateRecord(countQty + record.countQty, badQty + record.badQty)
    }

  }

  def processProduct(record: Record) {

    def createNewRecord() {
      val statement = connection.prepareStatement("INSERT INTO product (product, count_qty, bad_qty) VALUES(?, ?, ?)");
      statement.setString(1, record.product)
      statement.setLong(2, record.countQty);
      statement.setLong(3, record.badQty);
      statement.executeUpdate();
      statement.close();
    }

    def updateRecord(countQty: Long, badQty: Long) {
      val statement = connection.prepareStatement("UPDATE product SET count_qty=?, bad_qty =? WHERE product=?");
      statement.setLong(1, countQty);
      statement.setLong(2, badQty);
      statement.setString(3, record.product);
      statement.executeUpdate();
      statement.close();
    }

    def getData(record: Record): Option[(Long, Long)] = {
      var data: Option[(Long, Long)] = None
      val statement = connection.prepareStatement("SELECT count_qty, bad_qty FROM product WHERE product=?");
      statement.setString(1, record.product);
 
      val result = statement.executeQuery();
      if (result.next) {
        data = Some((result.getLong(1), result.getLong(2)))
      }
      statement.close();
      result.close();
      data
    }

    getData(record) match {
      case None => createNewRecord()
      case Some((countQty, badQty)) => updateRecord(countQty + record.countQty, badQty + record.badQty)
    }

  }

  def processMonthly(record: Record) {

    val dateFormatter = new SimpleDateFormat("yyyy-MM")
    val dateTimestamp = dateFormatter.format(new Date(record.embDate * 1000))

    def createNewRecord() {
      val statement = connection.prepareStatement("INSERT INTO monthly (timestamp, mach_id, defact_id, count_qty, bad_qty) VALUES(?, ?, ?, ?, ?)");
      statement.setString(1, dateTimestamp);
      statement.setString(2, record.machID);
      statement.setLong(3, record.defactID);
      statement.setLong(4, record.countQty);
      statement.setLong(5, record.badQty);
      statement.executeUpdate();
      statement.close();
    }

    def updateRecord(countQty: Long, badQty: Long) {
      val statement = connection.prepareStatement("UPDATE monthly SET count_qty=?, bad_qty =? WHERE timestamp=? AND mach_id=? AND defact_id=?");
      statement.setLong(1, countQty);
      statement.setLong(2, badQty);
      statement.setString(3, dateTimestamp);
      statement.setString(4, record.machID);
      statement.setLong(5, record.defactID);
      statement.executeUpdate();
      statement.close();
    }

    def getData(record: Record): Option[(Long, Long)] = {
      var data: Option[(Long, Long)] = None
      val statement = connection.prepareStatement("SELECT count_qty, bad_qty FROM monthly WHERE timestamp=? AND mach_id=? AND defact_id=?");
      statement.setString(1, dateTimestamp)
      statement.setString(2, record.machID);
      statement.setLong(3, record.defactID);
 
      val result = statement.executeQuery();
      if (result.next) {
        data = Some((result.getLong(1), result.getLong(2)))
      }
      statement.close();
      result.close();
      data
    }

    getData(record) match {
      case None => createNewRecord()
      case Some((countQty, badQty)) => updateRecord(countQty + record.countQty, badQty + record.badQty)
    }

  }

  def addData(record: Record) {
    processDaily(record)
    processInterval(record)
    processProductDaily(record)
    processProduct(record)
    processMonthly(record)
  }

}

object Processing {



  def main(args: Array[String]) {

    Class.forName("org.postgresql.Driver");

    val connection = DriverManager.getConnection("jdbc:postgresql://localhost:5432/zhenhai?charSet=utf-8", "zhenhai", "zhenhai123456")
    val file = Source.fromFile(new File("/home/brianhsu/dataSet2/x00"));
    var recordCount: Long = 0
    val addToPostgres = new AddToPostgres(connection)

    file.getLines().take(9000).foreach{line =>
      val record = Record(line)
      println(s"Processing [$recordCount]... $record")
      addToPostgres.addData(record)
      recordCount += 1;
    }
    val exporter = new Exporter(connection)
    exporter.export()
  }

}
