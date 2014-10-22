import AssemblyKeys._ // put this at the top of the file

assemblySettings

name := "CommunicationServer"

version := "0.0.1"

scalaVersion := "2.11.2"

scalacOptions += "-deprecation"

libraryDependencies ++= Seq(
  "com.rabbitmq" % "amqp-client" % "3.3.5",
  "org.mongodb" %% "casbah" % "2.7.3",
  "com.jsuereth" %% "scala-arm" % "1.4",
  "commons-daemon" % "commons-daemon" % "1.0.15"
)


