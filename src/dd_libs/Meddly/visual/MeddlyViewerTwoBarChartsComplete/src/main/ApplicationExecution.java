package main;

import info.ForestChangeInfo;
import info.ForestInfo;

import java.io.IOException;
import java.util.ArrayList;

import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.chart.BarChart;
import javafx.scene.chart.CategoryAxis;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.chart.XYChart.Series;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.scene.text.Text;
import javafx.stage.Stage;
import javafx.util.Duration;
import logging.LogSystem;
import logic.MEDDLYOutputParser;

public class ApplicationExecution extends Application {
	private static MEDDLYOutputParser applicationInfoParser;
	private static String fileToReadFrom = "k5n.txt";
	private static int logScaleUpperBound = 10000;
	private boolean stopOnNextPLine = false;
	private boolean firstTime = true;
	private static boolean debug = true;
	static LogSystem log;

	public static void main(String[] args) {
		log = new LogSystem(debug);
		log.log("Initalization Started");
		try {
			handleArgs(args);
			applicationInfoParser = new MEDDLYOutputParser(fileToReadFrom, log);
			launch(args); // Calls start()
			log.log("Program Execution Completed Successfully");
		} catch (Exception e) {
			log.logError("Exiting Main Method, An Error Occured");
			e.printStackTrace();
		}
	}

	@SuppressWarnings("unchecked")
	@Override
	public void start(Stage stage) {

		// Build Initial control buttons and set up bar chart axis
		final LogarithmicAxis xAxis = new LogarithmicAxis(1, logScaleUpperBound);
		final CategoryAxis yAxis = new CategoryAxis();
		final BarChart<Number, String> bc = new BarChart<Number, String>(xAxis,
				yAxis);
		Timeline tl = new Timeline();
		ArrayList<ForestInfo> forestInformationArray = null;
		ArrayList<Series> forestInformationSeries = null;
		Text status = new Text(10, 50, "No Update");
		Button btnStart = new Button("Start");
		Button btnStop = new Button("Stop");
		Button btnStepToNewStatus = new Button("Run To Next Status");

		// Define properties for the bc and the buttons
		bc.setTitle("Summary of Forest Count");
		bc.setAnimated(true);
		bc.setMinHeight(600);
		btnStart.setMaxSize(150, 50);
		btnStop.setMaxSize(150, 50);
		btnStepToNewStatus.setMaxSize(150, 50);
		xAxis.setLabel("Number of nodes");
		xAxis.setTickLabelRotation(90);
		yAxis.setLabel("Forest Level");
		status.setFont(new Font(20));

		ComboBox<Double> timerOptions = createTimerOptions(100, 150, 200, 250,
				300);

		// Define handlers for control buttons
		btnStart.setOnAction(new EventHandler<ActionEvent>() {
			@Override
			public void handle(ActionEvent arg0) {
				if (firstTime == true)
					firstTime = false;
				tl.play();
			}

		});

		btnStop.setOnAction(new EventHandler<ActionEvent>() {
			@Override
			public void handle(ActionEvent arg0) {
				tl.stop();

			}

		});

		btnStart.setOnAction(new EventHandler<ActionEvent>() {
			@Override
			public void handle(ActionEvent arg0) {
				tl.play();
			}

		});

		btnStepToNewStatus.setOnAction(new EventHandler<ActionEvent>() {
			public void handle(ActionEvent arg) {
				if (!stopOnNextPLine)
					stopOnNextPLine = true;

				tl.play();
			}
		});

		// Data Parse Block and initialize series for timeline building
		try {
			forestInformationArray = applicationInfoParser.getAllForestInfo();
			forestInformationSeries = applicationInfoParser
					.setSeriesBasedOnForestInfo(forestInformationArray);
		} catch (IOException e) {
			e.printStackTrace();
			return;
		}
		// Animation Block - Each KeyFrame represents a new scene in the
		// timeline.
		createAnimatedTimeline(timerOptions, tl, bc, status,
				forestInformationArray);
		VBox contentPane = new VBox();
		HBox controls = new HBox();
		controls.getChildren().addAll(timerOptions, btnStart, btnStop,
				btnStepToNewStatus, status);
		contentPane.getChildren().addAll(bc, controls);
		Scene scene = new Scene(contentPane, 800, 600);

		for (Series s : forestInformationSeries) {

			bc.getData().add(s);
		}
		// if(forestInformationSeries.size() > 0){
		// //
		// applicationInfoParser.setAxisTicksByLargestForest(forestInformationArray,
		// forestInformationSeries);
		// // }

		stage.setScene(scene);
		stage.show();
	}

	/**
	 * A wrapper function designed to allow the changing of the speed the
	 * visulizer displays. the main timeline handler method, handleTimeline, is
	 * contained and updated within this function
	 * 
	 * @param timeOptions
	 * @param timedTimeLine
	 * @param bc
	 * @param status
	 * @param forestInformation
	 */
	private void createAnimatedTimeline(ComboBox<Double> timeOptions,
			Timeline timedTimeLine, BarChart<Number, String> bc, Text status,
			ArrayList<ForestInfo> forestInformation) {
		timedTimeLine.setCycleCount(Timeline.INDEFINITE);

		timeOptions.valueProperty().addListener(new ChangeListener<Double>() {
			@Override
			public void changed(ObservableValue<? extends Double> observable,
					Double oldValue, Double newValue) {
				handleTimeline(timedTimeLine, bc, status, newValue,
						forestInformation);
			}
		});
		timeOptions.getSelectionModel().select(0); // selects default speed of
													// 150ms.
	}

	/**
	 * The main display handler for the Timeline and Barchart
	 * 
	 * @param tl
	 *            The timeline to be updated with the keyframe
	 * @param bc
	 *            the BarChart of the visulizer, to be updated on the timeline.
	 * @param status
	 *            the current status of the parsing given from the MEDDLY output
	 *            P lines, if they exist.
	 * @param refreshRate
	 *            The rate at which the keyframe should complete.
	 * 
	 * @param forestInformation
	 *            The Forest information parsed from the MEDDLYOutputParser,
	 *            used to update the barchart at the proper levels.
	 */
	private void handleTimeline(Timeline tl, BarChart<Number, String> bc,
			Text status, double refreshRate,
			ArrayList<ForestInfo> forestInformation) {
		KeyFrame key = new KeyFrame(Duration.millis(refreshRate),
				new EventHandler<ActionEvent>() {
					@Override
					public void handle(ActionEvent actionEvent) {

						ObservableList<Series<Number, String>> seriesInformation = bc
								.getData();

						try {
							// Parse the updates for the chart from the
							// file stream
							ArrayList<ForestChangeInfo> currentInfoI = applicationInfoParser
									.parseLineFromFile();

							// Ensure the arraylist containing the values is
							// good.
							if (currentInfoI == null) {
								status.setText("Visualization Complete, No New Data");
								tl.stop();

							} else {
								// forest id of -1 means a phase change
								// has been reached and currentInfo is of size
								// 1. We Check and see if
								// the timeline should be stopped.
								if (currentInfoI.get(0).getID() == -1
										&& stopOnNextPLine) {
									tl.stop();
									stopOnNextPLine = false;
								}

								else
									for (ForestChangeInfo currentInfo : currentInfoI) {

										// Otherwise,
										// get the anc and the level for the
										// series to be updated
										int id = currentInfo.getID();
										int anc = currentInfo.getAnc();
										int level = currentInfo.getLevel();

										log.log("IN DISPLAY METHOD: ID: " + id
												+ "level: " + level + "anc: "
												+ anc);
										for (int i = 0; i < forestInformation
												.size(); i++) {
											if (id == forestInformation.get(i)
													.getId()) {
												level = level
														- (forestInformation
																.get(i)
																.getLeftCount());
												log.log("level after conversion is: "
														+ level);
												break;
											}

										}
										// Set the text of the status
										status.setText(applicationInfoParser
												.getStatus().toString());

										// Get the level of the bar chart to
										// update
										XYChart.Data<Number, String> levelOfBarChartToUpdate = (XYChart.Data<Number, String>) seriesInformation
												.get(id - 1).getData()
												.get((int) (level));

										// Apply the active node change to the
										// bar level
										levelOfBarChartToUpdate
												.setXValue(levelOfBarChartToUpdate
														.getXValue().intValue()
														+ anc);
										log.log("Updated barchart with values: id: "
												+ id
												+ " level: "
												+ level
												+ " anc: " + anc);
									}
							}
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
				});
		tl.stop();
		tl.getKeyFrames().setAll(key);
		if (!firstTime) {
			tl.play();
		}
	}

	/**
	 * Private argument handler for the Main Method
	 * 
	 * @param args
	 *            the arguments of the main method.
	 */
	private static void handleArgs(String[] args) {
		if (args.length > 0) {
			for (int i = 0; i < args.length; i++) {
				if (args[i].equals("-f"))
					fileToReadFrom = args[i + 1];

				if (args[i].equals("-ls"))
					logScaleUpperBound = Integer.parseInt(args[i + 1]);
				if (args[i].equals("-lg"))
					debug = Boolean.getBoolean(args[i + 1]);

				if (args[i].equals("-h")) {
					System.out
							.println("Format of command line is javac compiledProgram.java -f [filename]");
					System.out
							.println("optional arg: -ls [number] : sets the upperbound of the log axis ");
					System.out
							.println("optional arg: -lg [boolean] : false if debug info should be sent to a logfile, true if debug info should be displayed to System.Out. Defaults to true. ");
					System.out
							.println("Example use: java compiledProgram.java -f k1n.txt -ls 1000 ");
					return;
				}
			}
		}
	}

	/**
	 * Creates a ComboBox with the various speeds that the visulizer can display
	 * 
	 * @param options
	 *            A list of integers representing the speed (in ms) to update
	 *            the timeline.
	 * @return A combobox to be displayed with the various speeds
	 */
	private ComboBox<Double> createTimerOptions(double... options) {
		ObservableList<Double> data = FXCollections.observableArrayList();

		for (Double option : options) {
			data.add(option);
		}
		return new ComboBox<Double>(data);
	}

}
