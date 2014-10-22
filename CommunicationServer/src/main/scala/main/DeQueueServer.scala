package tw.com.zhenhai.main

import tw.com.zhenhai._
import tw.com.zhenhai.util._

object DeQueueServer {
  def main(args: Array[String]) = KeepRetry {

    val serverThread = new DeQueueServerThread
    serverThread.start()
    serverThread.join()
  }
}

