import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.Channel;
import com.rabbitmq.client.QueueingConsumer;
import java.sql.DriverManager

object Receiever {

  val QUEUE_NAME = "test"

  Class.forName("org.postgresql.Driver");

  def getConsumer = {
     val factory = new ConnectionFactory
     factory.setHost("localhost")
     val connection = factory.newConnection()
     val channel = connection.createChannel()
     channel.queueDeclare(QUEUE_NAME, true, false, false, null);
     val consumer = new QueueingConsumer(channel);

     channel.basicConsume(QUEUE_NAME, true, consumer);
     consumer
  }

  def main(args: Array[String]) {

     val connection = DriverManager.getConnection("jdbc:postgresql://localhost:5432/zhenhai?charSet=utf-8", "zhenhai", "zhenhai123456")
     val addToPostgres = new AddToPostgres(connection)
     val consumer = getConsumer

     var recordCount: Long = 0

     println(" [*] Waiting for messages. To exit press CTRL+C");

     while (true) {
       val delivery = consumer.nextDelivery();
       val message = new String(delivery.getBody());
       println(s" [$recordCount] Received '" + message + "'");
       addToPostgres.addData(Record(message))
       recordCount += 1;
     }

  }
}


