name := "ZhenhaiDashboard"

version := "0.0.1"

scalaVersion := "2.11.2"

scalacOptions += "-deprecation"

libraryDependencies ++= Seq(
  "com.rabbitmq" % "amqp-client" % "3.3.5",
  "org.mongodb" %% "casbah" % "2.7.3",
  "com.jsuereth" %% "scala-arm" % "1.4"
)


