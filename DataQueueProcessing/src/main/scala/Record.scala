case class Record(
  orderType: String, 
  lotNo: String, 
  workQty: Long, 
  countQty: Long,
  embDate: Long, 
  badQty: Long,
  machineIP: String,
  defactID: Long,
  machID: String,
  workID: String,
  cx: String,
  dx: String,
  lc: String,
  machineStatus: String,
  product: String
)

object Record {

  def apply(line: String) = {
    val columns = line.split(" ");
    val product = columns(1)
    new Record(
      columns(0), 
      columns(1), 
      columns(2).toLong, 
      columns(3).toLong,
      columns(4).toLong,
      columns(5).toLong, 
      columns(6),
      columns(7).toLong,
      columns(8),
      columns(9),
      columns(10),
      columns(11),
      columns(12),
      columns(13),
      product
    )
  }
}
