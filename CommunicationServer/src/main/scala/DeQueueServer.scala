import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.QueueingConsumer;

object DeQueueServer {

  val QueueName = "rawDataLine"

  def initRabbitMQ() = {
     val factory = new ConnectionFactory
     factory.setHost("localhost")
     val connection = factory.newConnection()
     val channel = connection.createChannel()
     channel.queueDeclare(QueueName, true, false, false, null);
     channel.basicQos(1)

     val consumer = new QueueingConsumer(channel);
     channel.basicConsume(QueueName, false, consumer);
     (channel, consumer)
  }

  def main(args: Array[String]) {

     val (channel, consumer) = initRabbitMQ()
     val mongoProcessor = new MongoProcessor

     var recordCount: Long = 0

     println(" [*] Waiting for messages. To exit press CTRL+C");

     while (true) {
       val delivery = consumer.nextDelivery();
       val message = new String(delivery.getBody());
       println(s" [$recordCount] DeQueue '" + message + "'");
       mongoProcessor.addRecord(Record(message))
       channel.basicAck(delivery.getEnvelope.getDeliveryTag, false);
       recordCount += 1;
     }

  }
}


