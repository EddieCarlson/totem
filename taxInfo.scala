trait External {
   def readFile(filename: String): List[String] = ???
}

object DataProcessor extends External {
  var amountOwed: Map[String, Int]

  def processFile(data_filename: String): Int = {
    val fileLines = readFile(data_filename)
    if (data_filename.endsWith(".csv") && data_filename.startsWith("data_report") && data_filename.contains("tax_information")) {
      processData(fileLines)
    } else {
      -1
    }
  }

  def processData(input: List[String]): Int = {
    var totalOwed: Int = 0
    for (item <- input) {
      if (item.nonEmpty) {
        if (item.contains(":")) {
          val name = item.substring(0, item.indexOf(':'));
          val amount = item.substring(item.indexOf(':')).toInt
          amountOwed = amountOwed.updated(name, amount);
          totalOwed = totalOwed + amountOwed(name)
        }
      }
    }
    totalOwed
  }
}
