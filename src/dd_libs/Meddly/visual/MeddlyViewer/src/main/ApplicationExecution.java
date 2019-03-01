
// $Id$

/*
    Meddly: Multi-terminal and Edge-valued Decision Diagram LibrarY.
    Copyright (C) 2009, Iowa State University Research Foundation, Inc.

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

package main;

import info.LeafInfo;

import java.io.IOException;
import java.util.ArrayList;

import javafx.animation.Animation;
import javafx.animation.KeyFrame;
import javafx.animation.Timeline;
import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.Scene;
import javafx.scene.chart.BarChart;
import javafx.scene.chart.CategoryAxis;
import javafx.scene.chart.NumberAxis;
import javafx.scene.chart.XYChart;
import javafx.scene.control.Button;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;
import javafx.scene.text.Text;
import javafx.stage.Stage;
import javafx.util.Duration;
import logic.ForestInfoParser;

public class ApplicationExecution extends Application {
	private static ForestInfoParser applicationInfoParser = new ForestInfoParser();

	public static void main(String[] args) {
			// throws org.json.simple.parser.ParseException {
		launch(args);

	}
	
	@SuppressWarnings("unchecked")
	@Override
	public void start(Stage stage) {

		// Build Initial control buttons and set up bar chart axis
		final NumberAxis xAxis = new NumberAxis();
		final CategoryAxis yAxis = new CategoryAxis();
		final BarChart<Number, String> bc = new BarChart<Number, String>(xAxis,
				yAxis);
		Timeline tl = new Timeline();
		bc.setTitle("Summary of Forest Count");
		bc.setAnimated(true);
		bc.setMinHeight(600);
		Button btnStart = new Button("Start");
		Button btnStop = new Button("Stop");
		Button btnStepToNewStatus = new Button("Run To Next Status");
		btnStart.setMaxSize(150, 50);
		btnStop.setMaxSize(150, 50);
		btnStepToNewStatus.setMaxSize(150, 50);
		xAxis.setLabel("Number of nodes");
		xAxis.setTickLabelRotation(90);
		yAxis.setLabel("Forest Level");
		XYChart.Series series = null;
		Text status = new Text(10, 50, "No Update");
		status.setFont(new Font(20));
		
		
		
		
		// Define handlers for control buttons
		btnStart.setOnAction(new EventHandler<ActionEvent>() {
			@Override
			public void handle(ActionEvent arg0) {
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


		// Data Parse Block and initialize series for timeline building
		try {

			series = applicationInfoParser.initalizeForestInfo();

		} catch (IOException e) {
			e.printStackTrace();
		}

		
		// Animation Block - Each KeyFrame represents a new scene in the timeline. 
		tl.getKeyFrames().addAll(
				new KeyFrame(Duration.millis(150),
						new EventHandler<ActionEvent>() {
							@Override
							public void handle(ActionEvent actionEvent) {
								
								// Get the proper series from the bar chart
								//TODO: Allow for multiple series using forest ID
								XYChart.Series<Number, String> series = bc.getData().get(0);

								try {
									// Parse the updates for the chart from the file stream
									ArrayList<Integer> currentInfo = applicationInfoParser.parseNodeInfo();
									
									// Ensure the arraylist containing the values is populated
									if (currentInfo.size() > 0) {
										
										// get the anc and the level for the series to be updated
										int anc = currentInfo.get(1);
										int level = currentInfo.get(2);
										
										// Set the text of the status
										status.setText(applicationInfoParser.getStatus().toString());

										// Get the level of the bar chart to update
										XYChart.Data<Number, String> levelOfBarChartToUpdate = (XYChart.Data<Number, String>) series.getData().get((int) (level - 1));
										
										// Apply the active node change to the bar level
										levelOfBarChartToUpdate.setXValue(levelOfBarChartToUpdate.getXValue().intValue() + anc);
																				
									} else {
										status.setText("Visualization Complete, No New Data");
										tl.stop();
									}
								} catch (Exception e) {
									e.printStackTrace();
								}
							}
						}));
		
		tl.setCycleCount(Animation.INDEFINITE);

		VBox contentPane = new VBox();
		contentPane.getChildren().addAll(bc, btnStart, btnStop, status);
		Scene scene = new Scene(contentPane, 800, 600);
		bc.getData().addAll(series);
		stage.setScene(scene);
		stage.show();
	}

	
}
