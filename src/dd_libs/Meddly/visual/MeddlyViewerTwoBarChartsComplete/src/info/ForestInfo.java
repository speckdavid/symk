package info;

import javafx.scene.chart.XYChart;
import javafx.scene.chart.XYChart.Series;

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

}
