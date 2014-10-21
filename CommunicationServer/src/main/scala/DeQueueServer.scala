package tw.com.zhenhai

import scala.concurrent._
import ExecutionContext.Implicits.global

import tw.com.zhenhai.db.MongoProcessor
import tw.com.zhenhai.model.Record
import tw.com.zhenhai.util.KeepRetry

import com.rabbitmq.client.ConnectionFactory
import com.rabbitmq.client.Connection
import com.rabbitmq.client.Channel
import com.rabbitmq.client.QueueingConsumer

object DeQueueServer {

  val QueueName = "rawDataLine"

  def initRabbitMQ() = {
     val factory = new ConnectionFactory
     factory.setHost("localhost")
     val connection = factory.newConnection()
     val channel = connection.createChannel()
     channel.queueDeclare(QueueName, true, false, false, null)
     channel.basicQos(10)

     val consumer = new QueueingConsumer(channel)
     channel.basicConsume(QueueName, false, consumer)
     (channel, consumer)
  }

  def main(args: Array[String]) = KeepRetry {

     val (channel, consumer) = initRabbitMQ()
     val mongoProcessor = new MongoProcessor

     var recordCount: Long = 0

     println(" [*] Start DeQueue Server to append to MongoDB.")

     while (true) {
       val delivery = consumer.nextDelivery()
       val message = new String(delivery.getBody())

       Future {
         println(s" [*] [$recordCount] DeQueue: $message")

         Record(message).foreach{ record =>
           record.countQty match {
             case -1 => mongoProcessor.addMachineAlert(record)
             case  n => mongoProcessor.addRecord(record)
           }
         }

         channel.basicAck(delivery.getEnvelope.getDeliveryTag, false)
         recordCount += 1
       }
     }

  }
}


