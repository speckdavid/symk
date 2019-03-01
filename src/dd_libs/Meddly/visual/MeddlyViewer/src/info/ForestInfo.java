
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

package info;

import javafx.scene.chart.XYChart;
import javafx.scene.chart.XYChart.Series;

// import org.json.simple.JSONObject;

/**
 * Forest Information Class used to build and update chart series information
 * for visualizer
 * 
 * @author Coleman
 */
public class ForestInfo {

	private int forestDepth;
	private String forestName;
	private int left;
	private int right;
	private int id;
	private Series series;

	public ForestInfo(int id2, String name, int leftCount, int forestDepth) {
		this.forestDepth = forestDepth;
		this.forestName = name;
		this.id = id2;
		left = leftCount;
		right = forestDepth;
		series = this.setSeries(this.forestDepth);
	}

	/**
	 * Sets the forest depth if necessary.
	 * 
	 * @param newForestDepth
	 *            A long representing the forest depth.
	 */
	public void setForestDepth(int newForestDepth) {
		this.forestDepth = newForestDepth;
	}

	/**
	 * 
	 * @param newForestName
	 */
	public void setForestName(String newForestName) {
		this.forestName = newForestName;
	}

	/**
	 * 
	 * @param left
	 */
	public void setLeftMostNode(int left) {
		this.left = left;
	}

	/**
	 * 
	 * @param right
	 */
	public void setRightMostNode(int right) {
		this.right = right;
	}

	/**
	 * 
	 * @param id
	 */
	public void setForestID(int id) {
		this.id = id;
	}

	/**
	 * 
	 * @return
	 */
	public int getForestDepth() {
		return forestDepth;
	}

	/**
	 * 
	 * @return
	 */
	public String getForestName() {
		return forestName;
	}

	/**
	 * 
	 * @return
	 */
	public int getRightCount() {
		return right;
	}

	/**
	 * 
	 * @return
	 */
	public int getLeftCount() {
		return left;
	}

	/**
	 * 
	 * @return
	 */
	public int getId() {
		return id;
	}

	/**
	 * 
	 * @param numberOfInitialNodes
	 * @return
	 */

	public Series setSeries(int numberOfInitialNodes) {
		// TODO: Remove this from set series, as series is no longer inherit to
		// the Forest Info Class.
		XYChart.Series series = new XYChart.Series();
		series.setName(this.getForestName());
		for (int i = 0; i < numberOfInitialNodes; i++) {
			series.getData().add(new XYChart.Data(0, "" + i));
		}

		return series;

	}

	/**
	 * 
	 * @return
	 */
	public Series getSeries() {
		return series;
	}

}
