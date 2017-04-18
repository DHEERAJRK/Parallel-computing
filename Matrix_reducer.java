import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Reducer;

public class Matrix_Reducer extends Reducer<Text, Text, Text, IntWritable>
{
	@Override
	protected void reduce(Text key, Iterable<Text> values,Context context)
						throws IOException, InterruptedException 
	{
		
		Configuration conf = context.getConfiguration();
		String dimension = conf.get("dimension");
		
		int dim = Integer.parseInt(dimension);
		
		int[] row = new int[dim]; 
		int[] col = new int[dim];
		
		for(Text val : values)
		{
			String[] entries = val.toString().split(",");
			if(entries[0].matches("a"))
			{
				int index = Integer.parseInt(entries[2].trim());
				row[index] = Integer.parseInt(entries[3].trim());
			}
			if(entries[0].matches("b"))
			{
				int index = Integer.parseInt(entries[1].trim());
				col[index] = Integer.parseInt(entries[3].trim());
			}
		}

		int total = 0;
		for(int i = 0 ; i < 5; i++)
		{
			total += row[i]*col[i];
		}
		System.out.println(key.toString() + "-" + total );
		context.write(key, new IntWritable(total));
	
	}
	
}