import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Mapper;

public class Matrix_Mapper extends Mapper<LongWritable,Text,Text,Text>
{
	
	@Override
	protected void map(LongWritable key, Text value,Context context)
						throws IOException, InterruptedException
	{
		System.out.println("Inside Map !");
		String line = value.toString();
		String[] entry = line.split(",");
		String sKey = "";
		String mat = entry[0].trim();
		
		String row, col;
		
		Configuration conf = context.getConfiguration();
		String dimension = conf.get("dimension");
		
		System.out.println("Dimension from Mapper = " + dimension);
		
		int dim = Integer.parseInt(dimension);
		
		
		if(mat.matches("a"))
		{
			for (int i =0; i < dim ; i++) 
			{
				row = entry[1].trim();
				sKey = row+i;
				System.out.println(sKey + "-" + value.toString());
				context.write(new Text(sKey),value);
			}
		}
		
		if(mat.matches("b"))
		{
			for (int i =0; i < dim ; i++)
			{
				col = entry[2].trim(); 
				sKey = i+col;
				System.out.println(sKey + "-" + value.toString());
				context.write(new Text(sKey),value);
			}
		}
		
	}
	

}