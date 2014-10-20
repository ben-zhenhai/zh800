package tw.com.zhenhai.util

object KeepRetry {

  def apply(block: => Any) {
    try {
      block
    } catch {
      case e: Exception =>
        e.printStackTrace()
        println("Error encountered, wait 1 second to retry...")
        Thread.sleep(1000)
        apply(block)
    }
  }

}
