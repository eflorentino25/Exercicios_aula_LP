import java.util.Scanner;

public class Lista_1_exercicio10 {

	public static void main(String[] args) {
			System.out.println("Coloque as duas notas de avaliação uma em seguida da outra.");
			Scanner lo_scann = new Scanner(System.in);
			int lv_nota1 = lo_scann.nextInt();
			int lv_nota2 = lo_scann.nextInt();
			float lv_media = ((float)( lv_nota1 + lv_nota2 ) / 2);
			if (lv_media >= 6) {
			System.out.println("Aluno aprovado");
			System.out.printf("Média: %.2f", lv_media );
		}else {
		    System.out.println("Aluno reprovado ");
			System.out.printf("Média: %.2f", lv_media );		
		}
		
		lo_scann.close();
	}
}
