package logic;

import info.ForestChangeInfo;
import info.ForestInfo;
import logging.*;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;

import javafx.scene.chart.CategoryAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.chart.XYChart.Series;

/**
 * A Parser class which parses the information from Meddly for animation.
 *
 */
public class MEDDLYOutputParser {

	private String status;
	private String nextLineAfterFLine = null;
	private static BufferedReader br = null;
	private StringBuilder leafInfoStringBuilder = new StringBuilder();
	private ArrayList<ForestInfo> forestInfoArray;
	private LogSystem log;

	/**
	 * Constructor
	 * 
	 * @throws FileNotFoundException
	 */
	public MEDDLYOutputParser(String filename, LogSystem log) {
		status = "Initalized";
		this.log = log;
		try {
			MEDDLYOutputParser.br = new BufferedReader(new FileReader(filename));

		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * The Main F line parser, parses all Forest Information from the MEDDLY
	 * output file. This method constructs an arraylist of ForestInfo Objects
	 * 
	 * @return An Arraylist of ForestInfo objects, each containing information
	 *         about the forests to be displayed
	 * @throws IOException
	 * 
	 */
	public ArrayList<ForestInfo> getAllForestInfo() throws IOException {
		ArrayList<ForestInfo> seriesInformation = new ArrayList<ForestInfo>();
		try {

			// FILETYPE PARSE BLOCK
			String fileType = br.readLine(); // does nothing yet

			br.readLine(); // consume known comment line TODO: LOOK HERE IF YOU
							// CHANGE THE FILE FORMAT. IF YOU HAVE AN ERROR ITS
							// HERE.

			log.log(fileType);// error checking

			String stringOfForestInfo = br.readLine();

			while (stringOfForestInfo.startsWith("F")) {
				ForestInfo forestInfo = parseForestLine(stringOfForestInfo);
				seriesInformation.add(forestInfo);
				// ensure the line after the F lines is saved and can be read
				stringOfForestInfo = br.readLine();
				if (!stringOfForestInfo.startsWith("F")) {
					nextLineAfterFLine = stringOfForestInfo;
					break;
				}
			}

		} catch (Exception e) {
			if (br != null)
				br.close();
			e.printStackTrace();
		}
		forestInfoArray = seriesInformation; // grab the reference for use
												// later, but return to main
												// program for execution.
		return seriesInformation;
	}

	/**
	 * A private helper method to parse an F line from the MEDDLY Data
	 * 
	 * @param stringOfForestInfo
	 *            The F line to be parsed
	 * @return A wrapper ForestInfo object containing the data for the Forest
	 */
	private ForestInfo parseForestLine(String stringOfForestInfo) {

		log.log("Record: " + stringOfForestInfo);
		// END OF FILETYPE PARSE BLOCK

		StringBuilder leftAndRightCountStringConverter = new StringBuilder();

		// ID PARSE BLOCK
		int id = Integer.parseInt(stringOfForestInfo.substring(2, 3)); // TODO:
																		// May
																		// fail
																		// if
																		// larger
																		// than
																		// 10...
		// END OF ID PARSE BLOCK

		// NAME PARSE BLOCK
		String name = "";
		int index = -1;
		if (stringOfForestInfo.charAt(4) == '"') {
			index = 5;
			while (stringOfForestInfo.charAt(index) != '"') {
				name += stringOfForestInfo.charAt(index);
				index++;
			}
			// If there isn't a " at index 4, then there is no name in the
			// file, so name it untitled, and continue.
		} else {
			name = "Untitled";
			index = 4; // index used to get locations of subsequent items
						// after the name of the tree.
		}
		// END OF NAME PARSE BLOCK

		// LEFT AND RIGHT INITIAL NODE PARSE BLOCK
		if (index != 4)
			index += 2; // if the index is not 4, then name was parsed and
		// we are on a ", so increment to the next char
		// index.

		// Use a string builder to build the strings for left and right
		// counts, length of string unknown.

		leftAndRightCountStringConverter.append(stringOfForestInfo
				.charAt(index));
		while (stringOfForestInfo.charAt(index + 1) != ' ') {
			index++;
			leftAndRightCountStringConverter.append(stringOfForestInfo
					.charAt(index));
		}

		int leftCount = Integer.parseInt(leftAndRightCountStringConverter
				.toString());

		leftAndRightCountStringConverter.delete(0,
				leftAndRightCountStringConverter.length());

		// End of left node parsing, beginning of right node parsing
		leftAndRightCountStringConverter.append(stringOfForestInfo
				.charAt(index += 2));
		while (stringOfForestInfo.charAt(index + 1) != ' ') {
			index++;
			leftAndRightCountStringConverter.append(stringOfForestInfo
					.charAt(index));
		}

		int forestDepth = Integer.parseInt(leftAndRightCountStringConverter
				.toString());

		log.log((char) leftCount);
		log.log((char) forestDepth);

		// END OF LEFT AND RIGHT INITIAL NODE PARSE BLOCK

		ForestInfo forestInfo = new ForestInfo(id, name, leftCount, forestDepth);
		return forestInfo;
	}

	/**
	 * Main Parse Function. Parses the information of all MEDDLY Stream Lines
	 * after F Line Parsing. Currently supported Lines are a lines, p lines, and
	 * future F lines (not updated).
	 * 
	 * @return an arrayList that is populated with at least one LeafInfo or null
	 *         if parsing fails
	 * @throws IOException
	 *             thrown when parsing fails in an unexpected way.
	 */
	public ArrayList<ForestChangeInfo> parseLineFromFile() throws IOException {
		ArrayList<ForestChangeInfo> listOfChanges = null;
		try {
			log.log("Parse Function now Parsing next line of file.");
			String sCurrentLine;
			if (nextLineAfterFLine != null) {
				sCurrentLine = nextLineAfterFLine;
				nextLineAfterFLine = null;
			} else {
				sCurrentLine = br.readLine();
			}

			// Check for unknown lines, as there is no further error checking in
			// the individual parse functions
			if (sCurrentLine != null) {
				log.log("Record:\t" + sCurrentLine);
				if (!sCurrentLine.startsWith("a")
						&& !sCurrentLine.startsWith("p")
						&& !sCurrentLine.startsWith("F")
						&& !sCurrentLine.startsWith("#")) {
					log.logError("Parse Function reached an unknown line. Program terminating.");
					br.close(); // close the reader as its the last line of the
								// file
				}
				if (sCurrentLine.startsWith("p")) {
					log.log("P line found, parsing Phase info.");
					ForestChangeInfo leafinfo = parsePhaseInfo(sCurrentLine);
					listOfChanges = new ArrayList<ForestChangeInfo>();
					listOfChanges.add(leafinfo);
				}

				if (sCurrentLine.startsWith("#")) {
					log.log("Comment Line Found, skipping.");
					ForestChangeInfo leafinfo = new ForestChangeInfo(1, 1, 0);
					listOfChanges = new ArrayList<ForestChangeInfo>();
					listOfChanges.add(leafinfo);
				}
				if (sCurrentLine.startsWith("a")) {
					log.log("a line found. Parsing anc info.");
					ArrayList<ForestChangeInfo> leafinfo = parseActiveNodeCountInfoChange(sCurrentLine);
					return leafinfo;
				}

				if (sCurrentLine.startsWith("F")) {
					log.log("New Forest Line found, no updates will be made this parse.");
				}
			}

		} catch (Exception e) {
			log.logError("A problem was found.");
			e.printStackTrace();
		}

		return listOfChanges;
	}

	/**
	 * Private Helper Function to the Main Parse Function. P Line Parser -
	 * Parses P line status updates from the MEDDLY stream
	 * 
	 * @param currentLine
	 *            The string of meddly file text after the P line
	 * @return a dummy @LeafInfo object with a -1 forest id to signify a p line
	 *         change to the Application Execution
	 */
	private ForestChangeInfo parsePhaseInfo(String currentLine) {
		ForestChangeInfo leafinfo = new ForestChangeInfo(1, 1, 0);
		status = currentLine.substring(2);
		log.log(status);
		return leafinfo;
	}

	/**
	 * Private Helper Function to the Main Parse Function. a Line Parser -
	 * Parses all information contained on a lines from MEDDLY stream
	 * 
	 * @param sCurrentLine
	 *            The a line to parse.
	 * @return
	 * 
	 */
	private ArrayList<ForestChangeInfo> parseActiveNodeCountInfoChange(
			String sCurrentLine) {

		StringBuilder sb = new StringBuilder();
		int triCount = 0;
		ArrayList<Integer> processData = new ArrayList<Integer>();
		ArrayList<ForestChangeInfo> toReturn = new ArrayList<ForestChangeInfo>();

		if (sCurrentLine.charAt(2) == ' ') {
			toReturn.add(new ForestChangeInfo(1, 1, 0));
			return toReturn;
		}

		for (int index = 2; index < sCurrentLine.length(); index++) {
			log.logLine("Char is: " + sCurrentLine.charAt(index));

			if (index == sCurrentLine.length() - 1) {
				sb.append(sCurrentLine.charAt(index));
				int ilanc = Integer.parseInt(sb.toString());
				log.log("end of the line, illanc is " + ilanc
						+ " about to be added to processData");
				processData.add(ilanc);
				log.log("processData has a length of: " + processData.size());
				ForestChangeInfo forestToAdd = new ForestChangeInfo(
						processData.get(0), processData.get(1),
						processData.get(2));
				sb = new StringBuilder();
				triCount = 0;
				toReturn.add(forestToAdd);

				break;

			}
			if (sCurrentLine.charAt(index) == ' ') {
				int ilanc = Integer.parseInt(sb.toString());
				log.log("there is a space, illanc is " + ilanc
						+ " about to be added to processData");
				processData.add(ilanc);
				sb = new StringBuilder();
				triCount++;
			} else {
				sb.append(sCurrentLine.charAt(index));
			}

			if (triCount == 3 || index == sCurrentLine.length() - 1) {
				log.log("tricount is three, length of array is: "
						+ processData.size());
				ForestChangeInfo forestToAdd = new ForestChangeInfo(
						processData.get(0), processData.get(1),
						processData.get(2));
				toReturn.add(forestToAdd);
				processData = new ArrayList<Integer>();
				triCount = 0;
			}

			else if (sCurrentLine.charAt(index) == 't') {
				break; // TODO: Implement
			}

		}

		return toReturn;
	}

	/**
	 * Getter for the status if required. Used for testing purposes.
	 * 
	 * @return A string representing the status given by a P line
	 */
	public String getStatus() {
		return status;
	}

	/**
	 * Generates the Series to be used for the barchart visualization in the
	 * Application Execution.
	 * 
	 * @param forestInformationSeries
	 * 
	 * @return
	 */
	public ArrayList<Series> setSeriesBasedOnForestInfo(
			ArrayList<ForestInfo> forestInformationSeries) {

		ArrayList<Series> completedSeriesGeneratedFromForestObjects = new ArrayList<Series>();

		// If there are more than two forest information objects
		// in the forestInformationSeries, then it means based on meddly
		// generations
		// the forest with the larger forest level range is generated second
		// So set the range of the first forest to match the range of the
		// second.
		// This assumes: The maximum number of forest objects to be generated is
		// 2 and
		// The forest with negative values is generated second.
		if (forestInformationSeries.size() > 1) {
			forestInformationSeries.get(0).setLeftMostNode(
					forestInformationSeries.get(1).getLeftCount());
			forestInformationSeries.get(0).setRightMostNode(
					forestInformationSeries.get(1).getRightCount());
			forestInformationSeries.get(0).setForestDepth(
					forestInformationSeries.get(1).getForestDepth());

		}

		// for each forestinformation in the array, get the number of initial
		// nodes
		// and build the series based of the node counts.
		for (ForestInfo forest : forestInformationSeries) {
			Series seriesGeneratedByForest = new XYChart.Series();
			int numberOfIntitialNodes;
			if (forest.getLeftCount() > 0) {
				numberOfIntitialNodes = forest.getForestDepth();
			} else {
				numberOfIntitialNodes = forest.getRightCount()
						- forest.getLeftCount() + 1;
			}

			seriesGeneratedByForest.setName(forest.getForestName());
			int baseCounterForYAxisTickGeneration = forest.getLeftCount();

			for (int i = 0; i < numberOfIntitialNodes; i++) {
				seriesGeneratedByForest.getData().add(
						new XYChart.Data(0, ""
								+ baseCounterForYAxisTickGeneration));
				baseCounterForYAxisTickGeneration++;
			}

			completedSeriesGeneratedFromForestObjects
					.add(seriesGeneratedByForest);

		}

		return completedSeriesGeneratedFromForestObjects;

	}
}