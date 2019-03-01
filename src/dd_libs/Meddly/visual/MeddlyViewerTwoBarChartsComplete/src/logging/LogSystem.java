package logging;

import java.io.FileNotFoundException;
import java.io.PrintStream;

/**
 * Logging Class for MEDDLY Visualizer
 * 
 * @author Coleman
 *
 */
public class LogSystem {
	
	private PrintStream fileToWrite;

	/**
	 * Constructor method for the LogSystem
	 * 
	 * @param writeToSystemOut
	 *            A boolean representing whether to write the logging output to
	 *            System.out or to a file. True for Logging to System.out, False
	 *            otherwise.
	 */
	public LogSystem(boolean writeToSystemOut) {

		try {
			if (!writeToSystemOut) {
				this.fileToWrite = new PrintStream("LogFile");
			} else
				this.fileToWrite = System.out;
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			this.fileToWrite = System.out;
			this.logError("Invalid File Name, Logging to System.out");
		}
	}

	/**
	 * Logs a line to the logfile destination
	 * 
	 * @param Message
	 *            the message to log
	 */
	public void log(String Message) {
		fileToWrite.println(Message);
	}

	/**
	 * Logs a char to the logfile destination
	 * 
	 * @param Message
	 *            The character to log
	 */
	public void log(char message) {
		fileToWrite.println(message);
	}

	/**
	 * Logs an Error Message to logfile destination
	 * 
	 * @param Message
	 *            the message to log.
	 */
	public void logError(String Message) {
		fileToWrite.println("Error: " + Message);
	}

	/**
	 * Logs a line without a newline character to the logfile destination
	 * 
	 * @param Message
	 *            The message to log.
	 */
	public void logLine(String Message) {
		fileToWrite.print(Message);
	}
}
